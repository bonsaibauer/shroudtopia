import { execFileSync } from "node:child_process";
import { cp, mkdir, rm, writeFile } from "node:fs/promises";
import path from "node:path";

const run = (command, args, env = {}) => execFileSync(command, args, {
  stdio: "inherit", env: { ...process.env, ...env }
});
run("node", ["docs/tools/validate-contracts.mjs"]);
run("node", ["docs/tools/generate-reference.mjs"]);
run("tsc", ["-p", "docs/tsconfig.json"]);
await rm("site", { recursive: true, force: true });
await mkdir("site", { recursive: true });
run("mdbook", ["build", "docs", "-d", "../site/en"], {
  MDBOOK_BOOK__LANGUAGE: "en", MDBOOK_BOOK__SRC: "src-en",
  MDBOOK_BOOK__TITLE: "Shroudtopia API Documentation",
  MDBOOK_OUTPUT__HTML__SITE_URL: "/shroudtopia/en/"
});
run("mdbook", ["build", "docs", "-d", "../site/de"], {
  MDBOOK_BOOK__LANGUAGE: "de", MDBOOK_BOOK__SRC: "src-de",
  MDBOOK_BOOK__TITLE: "Shroudtopia API-Dokumentation",
  MDBOOK_OUTPUT__HTML__SITE_URL: "/shroudtopia/de/"
});
await cp(path.join("docs", "redirect.html"), path.join("site", "index.html"));
await writeFile(path.join("site", ".nojekyll"), "");
console.log("Built bilingual documentation in site/en and site/de.");
