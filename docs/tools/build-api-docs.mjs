import { mkdir, readFile, writeFile } from "node:fs/promises";
import path from "node:path";

const root = process.cwd();
const model = JSON.parse(await readFile(path.join(root, "docs", "generated", "api-model.json"), "utf8"));
const snapshot = JSON.parse(await readFile(path.join(root, "docs", "api", "examples", "real-assets.json"), "utf8"));
const outputRoot = path.join(root, "docs", "generated", "api");

const translations = {
  en: {
    title: "Shroudtopia API Documentation",
    description: `This page documents the native Shroudtopia C API 1.1. The operation paths organize native function calls for the ReDoc layout; they are not HTTP endpoints.

Every operation contains its exact C signature, parameter schema, result contract, and a C++ call example. The Asset section also includes static responses captured from real local Enshrouded KFC data. The documentation never connects to a local game process.`,
    native: "Native C function", response: "API result", real: "Real Enshrouded asset snapshot",
    realDescription: "Static values captured from the locally installed Enshrouded KFC files on 2026-09-17. No executable or KFC file is included.",
    language: "Language", source: "Canonical header"
  },
  de: {
    title: "Shroudtopia API-Dokumentation",
    description: `Diese Seite dokumentiert die native Shroudtopia-C-API 1.1. Die Operationspfade ordnen native Funktionsaufrufe für das ReDoc-Layout; sie sind keine HTTP-Endpunkte.

Jede Operation enthält die exakte C-Signatur, das Parameterschema, den Result-Vertrag und ein C++-Aufrufbeispiel. Der Asset-Bereich enthält außerdem statische Antworten aus echten lokalen Enshrouded-KFC-Daten. Die Dokumentation verbindet sich niemals mit einem lokalen Spielprozess.`,
    native: "Native C-Funktion", response: "API-Ergebnis", real: "Echter Enshrouded-Asset-Snapshot",
    realDescription: "Statische Werte aus den lokal installierten Enshrouded-KFC-Dateien, erfasst am 17.09.2026. Es wird keine EXE- oder KFC-Datei eingebunden.",
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
components.schemas.RealAssetSnapshot = {
  type: "object",
  description: translations.en.realDescription,
  example: { list_assets: snapshot.listAssets, recipe_registry: snapshot.recipeRegistry }
};

for (const locale of ["en", "de"]) {
  const l = translations[locale];
  const paths = {};
  for (const service of model.services) {
    for (const fn of functionsOf(service)) {
      const inputs = fn.parameters.filter(parameter => parameter.direction === "in");
      const outputs = fn.parameters.filter(parameter => parameter.direction === "out");
      const properties = Object.fromEntries(inputs.map(parameter => [parameter.name, {
        ...parameterSchema(parameter), description: parameter.description,
        ...(parameter.required === "conditional" ? { nullable: true } : {})
      }]));
      const responseExample = service.slug === "assets" && fn.name === "list_assets"
        ? snapshot.listAssets
        : { result: "RESULT_OK", result_code: 0, outputs: Object.fromEntries(outputs.map(parameter => [parameter.name, null])) };
      paths[`/${service.slug}/${fn.name}`] = {
        post: {
          tags: [locale === "de" ? service.title_de : service.title],
          summary: fn.summary,
          description: `**${l.native}:** \`api->${fn.name}\`\n\n**${l.source}:** \`api/shroudtopia.h\`\n\n\`\`\`c\n${fn.signature}\n\`\`\``,
          operationId: fn.name,
          requestBody: inputs.length ? {
            required: true,
            content: { "application/json": { schema: { type: "object", required: inputs.filter(parameter => parameter.required === "yes").map(parameter => parameter.name), properties } } }
          } : undefined,
          responses: {
            "200": {
              description: l.response,
              content: { "application/json": { schema: { $ref: "#/components/schemas/ResultEnvelope" }, example: responseExample } }
            }
          },
          "x-codeSamples": [{ lang: "C++", label: l.native, source: codeSample(fn) }]
        }
      };
    }
  }
  paths["/assets/real-data-snapshot"] = {
    get: {
      tags: [l.real], summary: l.real, description: l.realDescription, operationId: "realAssetSnapshot",
      responses: { "200": { description: l.real, content: { "application/json": { schema: { $ref: "#/components/schemas/RealAssetSnapshot" }, example: { list_assets: snapshot.listAssets, recipe_registry: snapshot.recipeRegistry } } } } }
    }
  };
  const spec = {
    openapi: "3.1.0",
    info: { title: l.title, version: model.apiVersion, description: l.description, license: { name: "MIT", url: "https://github.com/bonsaibauer/shroudtopia/blob/1.1.0/LICENSE" } },
    tags: [
      ...model.services.map(service => ({ name: locale === "de" ? service.title_de : service.title, description: locale === "de" ? service.summary_de : service.summary })),
      { name: l.real, description: l.realDescription }
    ],
    paths, components
  };
  const output = path.join(outputRoot, locale);
  await mkdir(output, { recursive: true });
  await writeFile(path.join(output, "openapi.json"), `${JSON.stringify(spec, null, 2)}\n`);
  const languageLinks = `<nav class="language" aria-label="${l.language}"><a class="${locale === "de" ? "active" : ""}" href="../../de/api/">DE · Deutsch</a><a class="${locale === "en" ? "active" : ""}" href="../../en/api/">EN · English</a></nav>`;
  const html = `<!doctype html><html lang="${locale}"><head><meta charset="utf-8"><title>${l.title}</title><meta name="viewport" content="width=device-width,initial-scale=1"><style>body{margin:0;padding:0}.language{display:flex;gap:8px;position:fixed;right:22px;top:14px;z-index:1000}.language a{background:#fff;border:1px solid #d5d9dc;border-radius:7px;color:#25313a;font:700 14px/1 Segoe UI,Arial,sans-serif;padding:11px 14px;text-decoration:none;box-shadow:0 2px 8px #0002}.language a.active{background:#38bdf8;border-color:#38bdf8;color:#07131a}@media(max-width:720px){.language{position:relative;right:auto;top:auto;background:#fff;padding:10px}.language a{flex:1;text-align:center}}</style></head><body>${languageLinks}<redoc spec-url="./openapi.json" lazy-rendering theme='{"logo":{"gutter":"20px"},"sidebar":{"width":"285px"},"rightPanel":{"backgroundColor":"#263238"}}'></redoc><script src="https://cdn.jsdelivr.net/npm/redoc/bundles/redoc.standalone.js"></script></body></html>`;
  await writeFile(path.join(output, "index.html"), html);
}

console.log("Built ReDoc API documentation with static real asset examples.");
