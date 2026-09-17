import { mkdir, readFile, readdir, writeFile } from "node:fs/promises";
import path from "node:path";

const root = process.cwd();
const headersDir = path.join(root, "api/include/shroudtopia/api");
const headers = (await readdir(headersDir)).filter((name) => name.endsWith(".h")).sort();

const text = {
  en: {
    title: "API reference",
    intro: "This reference is generated directly from the public headers. The declarations below are the canonical API; no separate contract copy is maintained.",
    source: "Canonical source",
    note: "Use the domain guides for workflows, lifecycle rules, permissions, and examples."
  },
  de: {
    title: "API-Referenz",
    intro: "Diese Referenz wird direkt aus den öffentlichen Headern erzeugt. Die folgenden Deklarationen sind die kanonische API; es wird keine separate Contract-Kopie gepflegt.",
    source: "Kanonische Quelle",
    note: "Abläufe, Lifecycle-Regeln, Berechtigungen und Beispiele stehen in den Domain-Guides."
  }
};

for (const language of ["en", "de"]) {
  const labels = text[language];
  const output = [
    "<!-- Generated from api/include/shroudtopia/api/*.h; do not edit by hand. -->",
    `# ${labels.title}`,
    "",
    labels.intro,
    "",
    `> ${labels.note}`,
    ""
  ];
  for (const name of headers) {
    const relative = `api/include/shroudtopia/api/${name}`;
    const source = (await readFile(path.join(headersDir, name), "utf8")).trim();
    output.push(`## \`${name}\``, "", `**${labels.source}:** \`${relative}\``, "", "```c", source, "```", "");
  }
  const outputDir = path.join(root, "docs/src/reference");
  await mkdir(outputDir, { recursive: true });
  await writeFile(path.join(outputDir, `api_${language}.md`), `${output.join("\n").trimEnd()}\n`);
}

console.log(`Generated the bilingual API reference from ${headers.length} public headers.`);
