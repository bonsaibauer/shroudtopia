import { readFile, readdir } from "node:fs/promises";
import path from "node:path";
import process from "node:process";

const root = process.cwd();
const errors = [];
const forbidden = ["ST_", "HostApiV1", "AssetsApiV1", "WorldApiV1", "CreateModV1", "ShroudtopiaCreateMod", "visit_resources", "read_resource_json", "replace_resource_json", "set_resource_field_json", "discard_changes", "query_entities", "get_entity_transform", "spawn_entity", "destroy_entity", "set_entity_transform", "read_grid_region", "write_grid_region", "execute_command", "get_setting_bool", "get_setting_number"];
const required = {
  "api/shroudtopia.h": ["Api", "ShroudtopiaGetApi", "register_service", "register_action", "get_mod_setting_bool", "create_patch", "list_assets", "get_asset", "update_asset", "set_asset_field", "save_assets", "create_text_window"]
};
const walk = async (directory) => (await Promise.all((await readdir(directory, { withFileTypes: true })).filter((entry) => entry.name !== "target").map(async (entry) => {
  const target = path.join(directory, entry.name);
  return entry.isDirectory() ? walk(target) : [target];
}))).flat();

for (const directory of ["api", "src", "mods", "tools"]) for (const file of await walk(path.join(root, directory))) {
  if (!/\.(?:h|c|cpp|json|py)$/.test(file)) continue;
  const source = await readFile(file, "utf8");
  for (const symbol of forbidden) {
    const found = symbol === "ST_" ? /\bST_[A-Za-z0-9_]+/.test(source) : source.includes(symbol);
    if (found) errors.push(`${path.relative(root, file)}: forbidden symbol ${symbol}`);
  }
}
for (const [file, symbols] of Object.entries(required)) {
  const source = await readFile(path.join(root, file), "utf8");
  for (const symbol of symbols) if (!source.includes(symbol)) errors.push(`${file}: missing required API symbol ${symbol}`);
}

const serviceFiles = (await readdir(path.join(root, "docs", "api", "services"))).filter((file) => file.endsWith(".yml"));
const slugs = new Set();
for (const file of serviceFiles) {
  let metadata;
  try { metadata = JSON.parse(await readFile(path.join(root, "docs", "api", "services", file), "utf8")); }
  catch (error) { errors.push(`docs/api/services/${file}: metadata must be JSON-compatible YAML (${error.message})`); continue; }
  for (const field of ["slug", "header", "title", "title_de", "summary", "summary_de", "status", "since", "threading", "capabilities"]) if (!(field in metadata)) errors.push(`docs/api/services/${file}: missing ${field}`);
  if (slugs.has(metadata.slug)) errors.push(`docs/api/services/${file}: duplicate slug ${metadata.slug}`);
  slugs.add(metadata.slug);
  if (metadata.header !== "shroudtopia.h") errors.push(`docs/api/services/${file}: header must be shroudtopia.h`);
}

const model = JSON.parse(await readFile(path.join(root, "docs", "generated", "api-model.json"), "utf8"));
if (model.services.length !== serviceFiles.length) errors.push("docs/generated/api-model.json: service count does not match metadata");
for (const locale of ["en", "de"]) {
  const contractPath = path.join(root, "docs", "generated", "api", locale, "openapi.json");
  try {
    const contract = JSON.parse(await readFile(contractPath, "utf8"));
    if (contract.openapi !== "3.1.0") errors.push(`${path.relative(root, contractPath)}: expected OpenAPI 3.1.0`);
    if (contract.paths["/assets/real-data-snapshot"]) errors.push(`${path.relative(root, contractPath)}: asset data must be documented on its operation, not as a snapshot endpoint`);
    if (contract.components.schemas.RealAssetSnapshot) errors.push(`${path.relative(root, contractPath)}: obsolete RealAssetSnapshot schema remains`);
    const operations = Object.values(contract.paths).flatMap(item => Object.values(item));
    for (const service of model.services) for (const fn of service.headers.flatMap(header => header.functions).filter(item => item.kind !== "callback")) {
      const operation = operations.find(item => item.operationId === fn.name);
      if (!operation) {
        errors.push(`${path.relative(root, contractPath)}: missing operation ${fn.name}`);
        continue;
      }
      const examples = operation.responses?.["200"]?.content?.["application/json"]?.examples;
      if (!examples?.result_ok) errors.push(`${path.relative(root, contractPath)}: ${fn.name} has no direct RESULT_OK response example`);
      for (const result of model.results) if (!examples?.[result.name.toLowerCase()]) errors.push(`${path.relative(root, contractPath)}: ${fn.name} has no direct ${result.name} response example`);
    }
    const references = JSON.stringify(contract).match(/#\/components\/schemas\/[A-Za-z0-9_]+/g) ?? [];
    for (const reference of new Set(references)) {
      const name = reference.split("/").at(-1);
      if (!contract.components.schemas[name]) errors.push(`${path.relative(root, contractPath)}: missing schema ${name}`);
    }
  } catch (error) {
    errors.push(`${path.relative(root, contractPath)}: invalid or missing ReDoc contract (${error.message})`);
  }
}
if (errors.length) { console.error(errors.map((error) => `- ${error}`).join("\n")); process.exit(1); }
console.log(`Validated public API naming, ${serviceFiles.length} service contracts, and ReDoc output.`);
