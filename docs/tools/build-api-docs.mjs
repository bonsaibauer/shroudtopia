import { mkdir, readFile, readdir, writeFile } from "node:fs/promises";
import path from "node:path";

const root = process.cwd();
const model = JSON.parse(await readFile(path.join(root, "docs", "generated", "api-model.json"), "utf8"));
const snapshot = JSON.parse(await readFile(path.join(root, "docs", "api", "examples", "real-assets.json"), "utf8"));
const outputRoot = path.join(root, "docs", "generated", "api");

const translations = {
  en: {
    title: "Shroudtopia API Documentation",
    description: `This page documents the native Shroudtopia C API 1.2. The operation paths organize native function calls for the ReDoc layout; they are not HTTP endpoints.

Every operation contains its exact C signature, parameter schema, result contract, and a C++ call example. The Asset section also includes static responses captured from real local Enshrouded KFC data. The documentation never connects to a local game process.`,
    native: "Native C function", response: "API result",
    realDescription: "The success response contains values captured from the locally installed Enshrouded KFC files on 2026-09-17. No executable or KFC file is included.",
    language: "Language", source: "Canonical header"
  },
  de: {
    title: "Shroudtopia API-Dokumentation",
    description: `Diese Seite dokumentiert die native Shroudtopia-C-API 1.2. Die Operationspfade ordnen native Funktionsaufrufe für das ReDoc-Layout; sie sind keine HTTP-Endpunkte.

Jede Operation enthält die exakte C-Signatur, das Parameterschema, den Result-Vertrag und ein C++-Aufrufbeispiel. Der Asset-Bereich enthält außerdem statische Antworten aus echten lokalen Enshrouded-KFC-Daten. Die Dokumentation verbindet sich niemals mit einem lokalen Spielprozess.`,
    native: "Native C-Funktion", response: "API-Ergebnis",
    realDescription: "Die erfolgreiche Antwort enthält Werte aus den lokal installierten Enshrouded-KFC-Dateien, erfasst am 17.09.2026. Es wird keine EXE- oder KFC-Datei eingebunden.",
    language: "Sprache", source: "Kanonischer Header"
  }
};

const functionsOf = service => service.headers.flatMap(header => header.functions).filter(fn => fn.kind !== "callback");
const schemaName = type => type.replace(/\bconst\b/g, "").replaceAll("*", "").trim();
const knownTypes = new Set(model.services.flatMap(service => service.headers.flatMap(header => [
  ...header.structs.map(structure => structure.name), ...header.enums.map(enumeration => enumeration.name)
])));
const parameterSchema = parameter => {
  const type = schemaName(parameter.type);
  if (type === "StringView") return { type: "string" };
  if (/^(u?int\d+_t|size_t|Registration|RuntimePatch|TextWindow)$/.test(type)) return { type: "integer" };
  if (type === "double" || type === "float") return { type: "number" };
  if (type === "void") return {};
  return knownTypes.has(type) ? { $ref: `#/components/schemas/${type}` } : { type: "string", description: `Native type: ${type}` };
};
const argument = parameter => {
  const values = {
    owner_id: 'View("mod.example")', action_id: 'View("mod.example.action")',
    command_id: 'View("mod.example.command")', event_id: 'View("mod.example.event")',
    type_name: 'View("keen::RecipeRegistryResource")', path: 'View("/recipes/0/requiredEnergy")',
    json: 'View("0")'
  };
  if (values[parameter.name]) return values[parameter.name];
  if (parameter.direction === "out" || parameter.type.includes("*")) return `&${parameter.name}`;
  if (parameter.type.includes("StringView")) return 'View("value")';
  return parameter.name;
};
const codeSample = fn => `Result result = api->${fn.name}(${fn.parameters.map(argument).join(", ")});\nif (result != RESULT_OK) {\n    return result;\n}`;

const resultSchema = {
  type: "object",
  required: ["result", "result_code", "outputs"],
  properties: {
    result: { type: "string", enum: model.results.map(result => result.name), example: "RESULT_OK" },
    result_code: { type: "integer", example: 0 },
    outputs: { type: "object", additionalProperties: true }
  }
};

const components = { schemas: {} };
for (const service of model.services) {
  for (const header of service.headers) {
    for (const structure of header.structs) {
      components.schemas[structure.name] = {
        type: "object",
        required: structure.fields.map(field => field.name),
        properties: Object.fromEntries(structure.fields.map(field => [field.name, parameterSchema({ type: field.type })]))
      };
    }
    for (const enumeration of header.enums) {
      components.schemas[enumeration.name] = {
        type: "string",
        enum: enumeration.values.map(value => value.name),
        description: enumeration.values.map(value => `${value.name} = ${value.value}`).join("\n\n")
      };
    }
  }
}
components.schemas.ResultEnvelope = resultSchema;
const realAssetDocument = {
  inputCategories: [],
  recipes: [snapshot.recipeRegistry.firstRecipe]
};
const successOutputs = {
  register_action: { registration: 1001 },
  invoke_action: {},
  get_action_state: { state: "ACTION_AVAILABLE" },
  ShroudtopiaGetApi: { api: { struct_size: 264, api_version: 65537 } },
  list_assets: snapshot.listAssets.outputs,
  get_asset: {
    buffer: JSON.stringify(realAssetDocument),
    required_size: Buffer.byteLength(JSON.stringify(realAssetDocument), "utf8") + 1
  },
  update_asset: {},
  create_asset: { asset: snapshot.recipeRegistry.asset },
  reset_assets: {},
  save_assets: {},
  set_asset_field: {},
  query_capability: {
    information: {
      struct_size: 32,
      version_major: 1,
      version_minor: 1,
      flags: 0,
      available: 1,
      reserved: [0, 0, 0, 0, 0, 0, 0]
    }
  },
  check_permission: { allowed: 1 },
  register_command: { registration: 1002 },
  invoke_command: {},
  subscribe_event: { registration: 1003 },
  publish_event: {},
  log: {},
  read_log_tail: {
    buffer: "[I 00:00:00,000] [shroudtopia] Starting 1.2.0\n",
    written: Buffer.byteLength("[I 00:00:00,000] [shroudtopia] Starting 1.2.0\n", "utf8")
  },
  create_patch: { patch: 1 },
  set_patch_enabled: {},
  get_patch_state: { state: { struct_size: 16, enabled: 1, reserved: [0, 0, 0, 0, 0, 0, 0] } },
  release_patch: {},
  register_service: { registration: 1004 },
  find_service: { interface_pointer: "0x000001F000001100" },
  release_registration: {},
  get_mod_setting_bool: { value: 1 },
  get_mod_setting_number: { value: 500.0 },
  create_text_window: { window: 1 },
  set_text_window_text: {},
  get_text_window_status: { status: "TEXT_READY" },
  destroy_text_window: {}
};

const successResponse = fn => fn.name === "list_assets"
  ? snapshot.listAssets
  : { result: "RESULT_OK", result_code: 0, outputs: successOutputs[fn.name] ?? {} };

const responseExamples = fn => Object.fromEntries(model.results.map((result, resultCode) => [
  result.name.toLowerCase(),
  {
    summary: result.name,
    value: resultCode === 0
      ? successResponse(fn)
      : { result: result.name, result_code: resultCode, outputs: {} }
  }
]));

for (const locale of ["en", "de"]) {
  const l = translations[locale];
  const paths = {};
  for (const service of model.services) {
    for (const fn of functionsOf(service)) {
      const inputs = fn.parameters.filter(parameter => parameter.direction === "in");
      const properties = Object.fromEntries(inputs.map(parameter => [parameter.name, {
        ...parameterSchema(parameter), description: parameter.description,
        ...(parameter.required === "conditional" ? { nullable: true } : {})
      }]));
      const usesRealAssetData = service.slug === "assets" && ["list_assets", "get_asset", "create_asset"].includes(fn.name);
      paths[`/${service.slug}/${fn.name}`] = {
        post: {
          tags: [locale === "de" ? service.title_de : service.title],
          summary: fn.summary,
          description: `**${l.native}:** \`api->${fn.name}\`\n\n**${l.source}:** \`api/shroudtopia.h\`${usesRealAssetData ? `\n\n${l.realDescription}` : ""}\n\n\`\`\`c\n${fn.signature}\n\`\`\``,
          operationId: fn.name,
          requestBody: inputs.length ? {
            required: true,
            content: { "application/json": { schema: { type: "object", required: inputs.filter(parameter => parameter.required === "yes").map(parameter => parameter.name), properties } } }
          } : undefined,
          responses: {
            "200": {
              description: l.response,
              content: { "application/json": { schema: { $ref: "#/components/schemas/ResultEnvelope" }, examples: responseExamples(fn) } }
            }
          },
          "x-codeSamples": [{ lang: "C++", label: l.native, source: codeSample(fn) }]
        }
      };
    }
  }
  const spec = {
    openapi: "3.1.0",
    info: { title: l.title, version: model.apiVersion, description: l.description, license: { name: "MIT", url: "https://github.com/bonsaibauer/shroudtopia/blob/1.2.0/LICENSE" } },
    tags: model.services.map(service => ({ name: locale === "de" ? service.title_de : service.title, description: locale === "de" ? service.summary_de : service.summary })),
    paths, components
  };
  const output = path.join(outputRoot, locale);
  await mkdir(output, { recursive: true });
  await writeFile(path.join(output, "openapi.json"), `${JSON.stringify(spec, null, 2)}\n`);
}

const locales = (await readdir(outputRoot, { withFileTypes: true })).filter(entry => entry.isDirectory()).map(entry => entry.name).sort();
for (const locale of locales) {
  const contract = JSON.parse(await readFile(path.join(outputRoot, locale, "openapi.json"), "utf8"));
  const displayNames = new Intl.DisplayNames([locale, "en"], { type: "language" });
  const languageLinks = `<nav class="language" aria-label="Language">${locales.map(language => `<a class="${language === locale ? "active" : ""}" href="../${language}/">${language.toUpperCase()} · ${displayNames.of(language) ?? language}</a>`).join("")}</nav>`;
  const html = `<!doctype html><html lang="${locale}"><head><meta charset="utf-8"><title>${contract.info.title}</title><meta name="viewport" content="width=device-width,initial-scale=1"><style>body{margin:0;padding:0}.language{display:flex;gap:8px;position:fixed;right:22px;top:14px;z-index:1000}.language a{background:#fff;border:1px solid #d5d9dc;border-radius:7px;color:#25313a;font:700 14px/1 Segoe UI,Arial,sans-serif;padding:11px 14px;text-decoration:none;box-shadow:0 2px 8px #0002}.language a.active{background:#38bdf8;border-color:#38bdf8;color:#07131a}@media(max-width:720px){.language{position:relative;right:auto;top:auto;background:#fff;padding:10px}.language a{flex:1;text-align:center}}</style></head><body>${languageLinks}<redoc spec-url="./openapi.json" lazy-rendering theme='{"logo":{"gutter":"20px"},"sidebar":{"width":"285px"},"rightPanel":{"backgroundColor":"#263238"}}'></redoc><script src="https://cdn.jsdelivr.net/npm/redoc/bundles/redoc.standalone.js"></script></body></html>`;
  await writeFile(path.join(outputRoot, locale, "index.html"), html);
}

console.log("Built ReDoc API documentation with response examples on every operation.");
