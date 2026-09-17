import { mkdir, readFile, readdir, writeFile } from "node:fs/promises";
import path from "node:path";

const root = process.cwd();
const status = { stable: "✅", experimental: "🧪", planned: "🚧", unavailable: "❌" };
const labels = {
  en: { status: "Status", version: "Version", provider: "Provider", capability: "Capability", header: "Header", types: "Types", fields: "Fields", values: "Values", field: "Field", type: "C type / value", meaning: "Meaning", rules: "Rules", functions: "Functions", since: "Since", permission: "Permission", threading: "Threading", ownership: "Ownership", side: "Side effects", parameters: "Parameters", direction: "Direction", required: "Required", results: "Results", result: "Result", example: "Example", yes: "yes", no: "no", none: "None" },
  de: { status: "Status", version: "Version", provider: "Provider", capability: "Capability", header: "Header", types: "Typen", fields: "Felder", values: "Werte", field: "Feld", type: "C-Typ / Wert", meaning: "Bedeutung", rules: "Regeln", functions: "Funktionen", since: "Seit", permission: "Berechtigung", threading: "Threading", ownership: "Ownership", side: "Seiteneffekte", parameters: "Parameter", direction: "Richtung", required: "Pflicht", results: "Ergebnisse", result: "Ergebnis", example: "Beispiel", yes: "ja", no: "nein", none: "Keine" }
};
const esc = (value = "") => String(value).replaceAll("|", "\\|").replaceAll("\n", " ");
const local = (value, language) => value?.[language] ?? "";

for (const file of (await readdir(path.join(root, "docs/contracts"))).filter((name) => name.endsWith(".yaml")).sort()) {
  const contract = JSON.parse(await readFile(path.join(root, "docs/contracts", file), "utf8"));
  for (const language of ["en", "de"]) {
    const l = labels[language];
    const out = [`<!-- Generated from docs/contracts/${file}; do not edit by hand. -->`, `# ${local(contract.title, language)}`, "", local(contract.summary, language), "", `<div class="api-meta" data-api-status="${contract.status}">`, "", `- **${l.status}:** ${status[contract.status]} ${contract.status}`, `- **${l.version}:** \`${contract.version}\``, `- **${l.provider}:** ${local(contract.provider, language)}`, `- **${l.capability}:** ${contract.capability ? `\`${contract.capability}\`` : l.none}`, `- **${l.header}:** \`${contract.header}\``, "", "</div>", "", `## ${l.types}`, ""];
    for (const type of contract.types) {
      out.push(`### \`${type.name}\``, "", local(type.description, language), "");
      const items = type.fields ?? type.values;
      if (items?.length) {
        out.push(`| ${type.values ? l.values : l.field} | ${l.type} | ${l.meaning} | ${l.rules} |`, "|---|---|---|---|");
        for (const item of items) out.push(`| \`${esc(item.name)}\` | \`${esc(item.type)}\` | ${esc(local(item.description, language))} | ${esc(local(item.rules, language) || local(item.ownership, language) || "—")} |`);
        out.push("");
      }
    }
    out.push(`## ${l.functions}`, "");
    for (const fn of contract.functions) {
      out.push(`<section class="api-function" data-api-name="${fn.name.toLowerCase()}" data-api-status="${fn.status}">`, "", `### \`${fn.name}\``, "", local(fn.summary, language), "", "```c", fn.signature + ";", "```", "", `- **${l.status}:** ${status[fn.status]} ${fn.status}`, `- **${l.since}:** \`${fn.since}\``, `- **${l.permission}:** ${fn.permission ? `\`${fn.permission}\`` : l.none}`, `- **${l.threading}:** ${local(fn.threading, language)}`, `- **${l.ownership}:** ${local(fn.ownership, language)}`, `- **${l.side}:** ${local(fn.sideEffects, language)}`, "", `#### ${l.parameters}`, "", `| ${l.field} | ${l.type} | ${l.direction} | ${l.required} | ${l.meaning} |`, "|---|---|:---:|:---:|---|");
      for (const p of fn.parameters) out.push(`| \`${esc(p.name)}\` | \`${esc(p.type)}\` | ${p.direction} | ${p.required ? l.yes : l.no} | ${esc(local(p.description, language))} |`);
      out.push("", `#### ${l.results}`, "", `| ${l.result} | ${l.meaning} |`, "|---|---|");
      for (const result of fn.results) out.push(`| \`${result.code}\` | ${esc(local(result.description, language))} |`);
      out.push("", `#### ${l.example}`, "", "```cpp", fn.example, "```", "", "</section>", "");
    }
    const outputDir = path.join(root, `docs/src-${language}/reference/generated`);
    await mkdir(outputDir, { recursive: true });
    await writeFile(path.join(outputDir, file.replace(/\.yaml$/, ".md")), `${out.join("\n").trimEnd()}\n`);
  }
}
console.log("Generated English and German API references.");
