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
for (const service of model.services) {
  const reference = path.join(root, "docs", "content", "en", "reference", `${service.slug}.md`);
  try {
    const content = await readFile(reference, "utf8");
    for (const fn of service.headers.flatMap((header) => header.functions)) {
      if (!content.includes(`\`${fn.name}\``)) errors.push(`${path.relative(root, reference)}: missing function ${fn.name}`);
      for (const parameter of fn.parameters) if (!content.includes(`\`${parameter.name}\``)) errors.push(`${path.relative(root, reference)}: missing parameter ${fn.name}.${parameter.name}`);
    }
  } catch { errors.push(`${path.relative(root, reference)}: missing generated service reference`); }
}
if (errors.length) { console.error(errors.map((error) => `- ${error}`).join("\n")); process.exit(1); }
console.log(`Validated public API naming, ${serviceFiles.length} service contracts, and generated references.`);
