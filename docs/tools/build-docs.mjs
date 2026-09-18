import { execFileSync } from "node:child_process";
import { cp, mkdir, readdir, rm, writeFile } from "node:fs/promises";
import path from "node:path";

const run = (command, args) => execFileSync(command, args, { stdio: "inherit" });

run("node", ["docs/tools/generate-reference.mjs"]);
run("node", ["docs/tools/build-api-docs.mjs"]);
run("node", ["docs/tools/validate-api.mjs"]);

const generated = path.join("docs", "generated", "api");
const locales = (await readdir(generated, { withFileTypes: true }))
  .filter(entry => entry.isDirectory())
  .map(entry => entry.name)
  .sort();
if (!locales.includes("en")) throw new Error("The English ReDoc contract is required.");

await rm("site", { recursive: true, force: true });
await mkdir("site", { recursive: true });
for (const locale of locales) {
  await cp(path.join(generated, locale), path.join("site", locale), { recursive: true });
}
await cp(path.join("docs", "redirect.html"), path.join("site", "index.html"));
await writeFile(path.join("site", "languages.json"), `${JSON.stringify({ fallback: "en", languages: locales }, null, 2)}\n`);
await writeFile(path.join("site", ".nojekyll"), "");
console.log(`Built ReDoc for: ${locales.join(", ")}.`);
