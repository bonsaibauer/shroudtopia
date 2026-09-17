import { readFile, readdir } from "node:fs/promises";
import path from "node:path";
import process from "node:process";

const root = process.cwd();
const contentRoot = path.join(root, "docs", "content");
const errors = [];
const walk = async (directory) => (await Promise.all((await readdir(directory, { withFileTypes: true })).map(async (entry) => { const target = path.join(directory, entry.name); return entry.isDirectory() ? walk(target) : [target]; }))).flat();
const relativeMarkdown = async (language) => (await walk(path.join(contentRoot, language))).filter((file) => file.endsWith(".md")).map((file) => path.relative(path.join(contentRoot, language), file).replaceAll("\\", "/")).sort();
const english = await relativeMarkdown("en");
const locales = (await readdir(contentRoot, { withFileTypes: true })).filter((entry) => entry.isDirectory()).map((entry) => entry.name).sort();
for (const locale of locales.filter((item) => item !== "en")) {
  const translated = await relativeMarkdown(locale);
  for (const file of english.filter((file) => !translated.includes(file))) errors.push(`missing ${locale} translation: ${file}`);
  for (const file of translated.filter((file) => !english.includes(file))) errors.push(`missing English source for ${locale}: ${file}`);
}
const links = (source) => [...source.matchAll(/\]\((?!https?:|mailto:|#)([^)#]+)(?:#[^)]+)?\)/g)].map((match) => match[1]);
const codeBlocks = (source) => [...source.replaceAll("\r\n", "\n").matchAll(/```[^\n]*\n([\s\S]*?)```/g)].map((match) => match[1].trim());
for (const language of locales) for (const relative of await relativeMarkdown(language)) {
  const file = path.join(contentRoot, language, relative);
  for (const link of links(await readFile(file, "utf8"))) try { await readFile(path.resolve(path.dirname(file), link)); } catch { errors.push(`${path.relative(root, file)}: broken link ${link}`); }
}
for (const language of locales.filter((item) => item !== "en")) for (const relative of english.filter((file) => (locales.includes(language)))) {
  const translatedFiles = await relativeMarkdown(language);
  if (!translatedFiles.includes(relative)) continue;
  const enCode = codeBlocks(await readFile(path.join(contentRoot, "en", relative), "utf8"));
  const translatedCode = codeBlocks(await readFile(path.join(contentRoot, language, relative), "utf8"));
  if (enCode.length !== translatedCode.length) errors.push(`${relative}: code block count differs (${enCode.length} en, ${translatedCode.length} ${language})`);
  for (let index = 0; index < Math.min(enCode.length, translatedCode.length); index += 1) if (enCode[index] !== translatedCode[index]) errors.push(`${relative}: code block ${index + 1} differs in ${language}`);
}
const crowdin = await readFile(path.join(root, "crowdin.yml"), "utf8");
for (const required of ["preserve_hierarchy: true", "source: /docs/content/en/**/*.md", "translation: /docs/content/%two_letters_code%/**/%original_file_name%", "update_option: update_as_unapproved", "content_segmentation: 1"]) if (!crowdin.includes(required)) errors.push(`crowdin.yml: missing ${required}`);
if (/api_token|personal.token|project_id\s*:/.test(crowdin)) errors.push("crowdin.yml: VCS configuration must not contain credentials");
if (errors.length) { console.error(errors.map((error) => `- ${error}`).join("\n")); process.exit(1); }
console.log(`Validated ${english.length} English sources across ${locales.length} locales, links, code blocks, and Crowdin paths.`);
