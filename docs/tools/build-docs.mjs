import { execFileSync } from "node:child_process";
import { existsSync } from "node:fs";
import { cp, mkdir, readdir, rm, writeFile } from "node:fs/promises";
import path from "node:path";

const run = (command, args, env = {}) => execFileSync(command, args, { stdio: "inherit", env: { ...process.env, ...env } });
const cargoMdBook = process.env.USERPROFILE ? path.join(process.env.USERPROFILE, ".cargo", "bin", "mdbook.exe") : "";
const mdbook = cargoMdBook && existsSync(cargoMdBook) ? cargoMdBook : "mdbook";
const contentRoot = path.join("docs", "content");
const locales = (await readdir(contentRoot, { withFileTypes: true }))
  .filter((entry) => entry.isDirectory() && existsSync(path.join(contentRoot, entry.name, "SUMMARY.md")))
  .map((entry) => entry.name).sort();
if (!locales.includes("en")) throw new Error("docs/content/en/SUMMARY.md is required as the fallback locale");

run("node", ["docs/tools/generate-reference.mjs"]);
run("node", ["docs/tools/validate-api.mjs"]);
await rm("site", { recursive: true, force: true });
await mkdir("site", { recursive: true });
for (const locale of locales) {
  const display = new Intl.DisplayNames([locale], { type: "language" }).of(locale) ?? locale;
  run(mdbook, ["build", "docs", "-d", `../site/${locale}`], {
    MDBOOK_BOOK__LANGUAGE: locale,
    MDBOOK_BOOK__SRC: `content/${locale}`,
    MDBOOK_BOOK__TITLE: `Shroudtopia API — ${display}`,
    MDBOOK_OUTPUT__HTML__SITE_URL: `/shroudtopia/${locale}/`
  });
  const assets = path.join("site", locale, "assets");
  await mkdir(assets, { recursive: true });
  await cp(path.join("docs", "generated", "api-model.json"), path.join(assets, "api-model.json"));
  await cp(path.join("docs", "generated", "playground-schema.json"), path.join(assets, "playground-schema.json"));
  const uiFile = path.join(contentRoot, locale, "_ui.json");
  if (existsSync(uiFile)) await cp(uiFile, path.join(assets, "ui.json"));
}
await cp(path.join("docs", "redirect.html"), path.join("site", "index.html"));
await writeFile(path.join("site", "languages.json"), `${JSON.stringify({ fallback: "en", languages: locales }, null, 2)}\n`);
await writeFile(path.join("site", ".nojekyll"), "");
console.log(`Built documentation for: ${locales.join(", ")}.`);
