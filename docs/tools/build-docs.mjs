import { execFileSync } from "node:child_process";
import { cp, mkdir, readdir, readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";

const run = (command, args, env = {}) => execFileSync(command, args, {
  stdio: "inherit", env: { ...process.env, ...env }
});
run("node", ["docs/tools/generate-reference.mjs"]);
run("node", ["docs/tools/validate-api.mjs"]);
run("tsc", ["-p", "docs/tsconfig.json"]);
await rm("site", { recursive: true, force: true });
await mkdir("site", { recursive: true });

const sourceRoot = path.join("docs", "src");
const stageLanguage = async (language) => {
  const targetRoot = path.join("docs", ".build", language);
  await rm(targetRoot, { recursive: true, force: true });
  const visit = async (directory) => {
    for (const entry of await readdir(directory, { withFileTypes: true })) {
      const source = path.join(directory, entry.name);
      if (entry.isDirectory()) await visit(source);
      else if (entry.name.endsWith(`_${language}.md`)) {
        const relative = path.relative(sourceRoot, source).replace(new RegExp(`_${language}\\.md$`), ".md");
        const target = path.join(targetRoot, relative);
        await mkdir(path.dirname(target), { recursive: true });
        await writeFile(target, await readFile(source));
      }
    }
  };
  await visit(sourceRoot);
};
await stageLanguage("en");
await stageLanguage("de");
run("mdbook", ["build", "docs", "-d", "../site/en"], {
  MDBOOK_BOOK__LANGUAGE: "en", MDBOOK_BOOK__SRC: ".build/en",
  MDBOOK_BOOK__TITLE: "Shroudtopia API Documentation",
  MDBOOK_OUTPUT__HTML__SITE_URL: "/shroudtopia/en/"
});
run("mdbook", ["build", "docs", "-d", "../site/de"], {
  MDBOOK_BOOK__LANGUAGE: "de", MDBOOK_BOOK__SRC: ".build/de",
  MDBOOK_BOOK__TITLE: "Shroudtopia API-Dokumentation",
  MDBOOK_OUTPUT__HTML__SITE_URL: "/shroudtopia/de/"
});
await cp(path.join("docs", "redirect.html"), path.join("site", "index.html"));
await writeFile(path.join("site", ".nojekyll"), "");
console.log("Built bilingual documentation in site/en and site/de.");
