import { readFile, readdir } from "node:fs/promises";
import path from "node:path";
import process from "node:process";

const root = process.cwd();
const errors = [];
const forbidden = [
  "ST_", "HostApiV1", "AssetsApiV1", "WorldApiV1", "CreateModV1", "ShroudtopiaCreateMod",
  "visit_resources", "read_resource_json", "replace_resource_json",
  "set_resource_field_json", "discard_changes", "query_entities",
  "get_entity_transform", "spawn_entity", "destroy_entity",
  "set_entity_transform", "read_grid_region", "write_grid_region",
  "execute_command", "get_setting_bool", "get_setting_number"
];
const required = {
  "api/include/shroudtopia/api/host.h": [
    "Api", "register_action", "invoke_action", "get_action_state",
    "register_mod_settings", "get_mod_setting_bool", "get_mod_setting_number",
    "get_game_setting", "stage_game_setting", "reset_game_setting"
  ],
  "api/include/shroudtopia/api/assets.h": ["AssetsApi", "list", "get", "create", "update", "set", "save", "reset"],
  "api/include/shroudtopia/api/world.h": [
    "WorldApi", "list_entities", "get_entity", "create_entity", "update_entity",
    "remove_entity", "get_grid", "get_grid_region", "update_grid_region"
  ]
};

const walk = async (directory) => (await Promise.all((await readdir(directory, { withFileTypes: true })).map(async (entry) => {
  const target = path.join(directory, entry.name);
  return entry.isDirectory() ? walk(target) : [target];
}))).flat();

for (const directory of ["api", "src", "mods", "tools"]) {
  for (const file of await walk(path.join(root, directory))) {
    if (!/\.(?:h|c|cpp|json|py)$/.test(file)) continue;
    const source = await readFile(file, "utf8");
    for (const symbol of forbidden) if (source.includes(symbol)) {
      errors.push(`${path.relative(root, file)}: forbidden legacy symbol ${symbol}`);
    }
  }
}

for (const [file, symbols] of Object.entries(required)) {
  const source = await readFile(path.join(root, file), "utf8");
  for (const symbol of symbols) if (!source.includes(symbol)) errors.push(`${file}: missing required API symbol ${symbol}`);
}

const docsRoot = path.join(root, "docs/src");
const pathsFor = async (language) => {
  const summary = await readFile(path.join(docsRoot, `SUMMARY_${language}.md`), "utf8");
  return [...summary.matchAll(/\]\((\.\/[^)#]+\.md)/g)].map((match) => match[1].slice(2)).sort();
};

const english = await pathsFor("en");
const german = await pathsFor("de");
if (JSON.stringify(english) !== JSON.stringify(german)) errors.push("English and German SUMMARY.md paths differ");

for (const language of ["en", "de"]) {
  const localized = (relative) => relative.replace(/\.md$/, `_${language}.md`);
  for (const relative of await pathsFor(language)) {
    try { await readFile(path.join(docsRoot, localized(relative))); }
    catch {
      if (relative !== "reference/api.md") errors.push(`docs/src/SUMMARY_${language}.md: missing ${localized(relative)}`);
    }
  }
  for (const markdown of (await walk(docsRoot)).filter((file) => file.endsWith(`_${language}.md`))) {
    const content = await readFile(markdown, "utf8");
    for (const match of content.matchAll(/\]\((?!https?:|mailto:|#)([^)#]+)(?:#[^)]+)?\)/g)) {
      const target = path.resolve(path.dirname(markdown), localized(match[1]));
      try { await readFile(target); }
      catch {
        if (!match[1].endsWith("reference/api.md")) {
          errors.push(`${path.relative(root, markdown)}: broken link ${match[1]}`);
        }
      }
    }
  }
}

const markdown = (await walk(docsRoot)).filter((file) => /_(?:en|de)\.md$/.test(file));
for (const file of markdown) {
  const counterpart = file.replace(/_(en|de)\.md$/, (_, language) => `_${language === "en" ? "de" : "en"}.md`);
  try { await readFile(counterpart); }
  catch { errors.push(`${path.relative(root, file)}: missing language counterpart`); }
}

if (errors.length) {
  console.error(errors.map((error) => `- ${error}`).join("\n"));
  process.exit(1);
}
console.log("Validated canonical headers, naming rules, and bilingual navigation.");
