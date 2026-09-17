import { readFile, readdir } from "node:fs/promises";
import path from "node:path";
import process from "node:process";

const root = process.cwd();
const contractDir = path.join(root, "docs/contracts");
const files = (await readdir(contractDir)).filter((file) => file.endsWith(".yaml")).sort();
const errors = [];
const schema = JSON.parse(await readFile(path.join(root, "docs/schema/api-contract.schema.json"), "utf8"));
const resolveRef = (reference) => reference.slice(2).split("/").reduce((value, key) => value[key], schema);
const validateSchema = (value, rule, at) => {
  if (rule.$ref) return validateSchema(value, resolveRef(rule.$ref), at);
  for (const nested of rule.allOf ?? []) validateSchema(value, nested, at);
  const expected = Array.isArray(rule.type) ? rule.type : rule.type ? [rule.type] : [];
  const actual = value === null ? "null" : Array.isArray(value) ? "array" : typeof value;
  if (expected.length && !expected.includes(actual)) {
    errors.push(`${at}: expected ${expected.join(" or ")}, got ${actual}`); return;
  }
  if (rule.enum && !rule.enum.includes(value)) errors.push(`${at}: value is outside the schema enum`);
  if (typeof value === "string") {
    if (rule.minLength && value.length < rule.minLength) errors.push(`${at}: string is too short`);
    if (rule.pattern && !(new RegExp(rule.pattern)).test(value)) errors.push(`${at}: string does not match ${rule.pattern}`);
  }
  if (actual === "array") {
    if (rule.minItems && value.length < rule.minItems) errors.push(`${at}: array is too short`);
    if (rule.items) value.forEach((item, index) => validateSchema(item, rule.items, `${at}[${index}]`));
  }
  if (actual === "object") {
    for (const key of rule.required ?? []) if (!(key in value)) errors.push(`${at}: missing ${key}`);
    for (const [key, child] of Object.entries(value)) {
      if (rule.properties?.[key]) validateSchema(child, rule.properties[key], `${at}.${key}`);
      else if (rule.additionalProperties === false) errors.push(`${at}: unsupported property ${key}`);
    }
  }
};
const requiredLocalized = (value, at) => {
  if (!value || typeof value.en !== "string" || !value.en.trim() ||
      typeof value.de !== "string" || !value.de.trim()) errors.push(`${at}: missing en/de text`);
};
const requiredKeys = (value, keys, at) => {
  for (const key of keys) if (!(key in value)) errors.push(`${at}: missing ${key}`);
};
const contracts = [];

for (const file of files) {
  let contract;
  try { contract = JSON.parse(await readFile(path.join(contractDir, file), "utf8")); }
  catch (error) { errors.push(`${file}: not valid YAML-compatible JSON: ${error.message}`); continue; }
  contracts.push({ file, contract });
  validateSchema(contract, schema, file);
  requiredKeys(contract, ["id", "version", "status", "header", "title", "summary", "types", "functions"], file);
  requiredLocalized(contract.title, `${file}.title`);
  requiredLocalized(contract.summary, `${file}.summary`);
  if (!/^[0-9]+\.[0-9]+$/.test(contract.version ?? "")) errors.push(`${file}.version: expected major.minor`);
  if (!Array.isArray(contract.types) || !Array.isArray(contract.functions)) errors.push(`${file}: types/functions must be arrays`);
  const source = await readFile(path.join(root, contract.header), "utf8");
  const allHeaders = (await Promise.all((await readdir(path.join(root, "api/include/shroudtopia/api")))
    .filter((name) => name.endsWith(".h"))
    .map((name) => readFile(path.join(root, "api/include/shroudtopia/api", name), "utf8")))).join("\n");
  for (const type of contract.types ?? []) {
    requiredKeys(type, ["name", "kind", "description"], `${file}.types`);
    requiredLocalized(type.description, `${file}.${type.name}.description`);
    if (!allHeaders.includes(type.name)) errors.push(`${file}: documented type ${type.name} is absent from public headers`);
    for (const item of [...(type.fields ?? []), ...(type.values ?? [])]) {
      requiredKeys(item, ["name", "type", "description"], `${file}.${type.name}`);
      requiredLocalized(item.description, `${file}.${type.name}.${item.name}`);
    }
  }
  for (const fn of contract.functions ?? []) {
    requiredKeys(fn, ["name", "signature", "since", "status", "summary", "threading", "ownership", "parameters", "results", "example"], `${file}.functions`);
    for (const key of ["summary", "threading", "ownership"]) requiredLocalized(fn[key], `${file}.${fn.name}.${key}`);
    if (!allHeaders.includes(fn.name)) errors.push(`${file}: documented function ${fn.name} is absent from public headers`);
    if (!fn.signature.includes(fn.name)) errors.push(`${file}.${fn.name}: signature does not contain name`);
    for (const parameter of fn.parameters ?? []) {
      requiredKeys(parameter, ["name", "type", "description", "direction", "required"], `${file}.${fn.name}.parameters`);
      requiredLocalized(parameter.description, `${file}.${fn.name}.${parameter.name}`);
    }
    for (const result of fn.results ?? []) {
      if (!/^ST_RESULT_/.test(result.code ?? "")) errors.push(`${file}.${fn.name}: invalid result code`);
      requiredLocalized(result.description, `${file}.${fn.name}.${result.code}`);
    }
  }
  if (!source.trim()) errors.push(`${file}: empty source header`);
}

const pathsFor = async (language) => {
  const summary = await readFile(path.join(root, `docs/src-${language}/SUMMARY.md`), "utf8");
  return [...summary.matchAll(/\]\((\.\/[^)#]+\.md)/g)].map((match) => match[1].slice(2)).sort();
};
try {
  const en = await pathsFor("en"); const de = await pathsFor("de");
  if (JSON.stringify(en) !== JSON.stringify(de)) errors.push("English and German SUMMARY.md paths differ");
  for (const language of ["en", "de"]) for (const relative of await pathsFor(language)) {
    try { await readFile(path.join(root, `docs/src-${language}`, relative)); }
    catch { errors.push(`docs/src-${language}/SUMMARY.md: missing ${relative}`); }
  }
  for (const language of ["en", "de"]) {
    const sourceRoot = path.join(root, `docs/src-${language}`);
    const walk = async (directory) => (await Promise.all((await readdir(directory, { withFileTypes: true })).map(async (entry) => {
      const target = path.join(directory, entry.name);
      return entry.isDirectory() ? walk(target) : [target];
    }))).flat();
    for (const markdown of (await walk(sourceRoot)).filter((file) => file.endsWith(".md"))) {
      const content = await readFile(markdown, "utf8");
      for (const match of content.matchAll(/\]\((?!https?:|mailto:|#)([^)#]+)(?:#[^)]+)?\)/g)) {
        try { await readFile(path.resolve(path.dirname(markdown), match[1])); }
        catch { errors.push(`${path.relative(root, markdown)}: broken link ${match[1]}`); }
      }
    }
  }
} catch (error) { errors.push(`navigation validation failed: ${error.message}`); }

if (errors.length) {
  console.error(errors.map((error) => `- ${error}`).join("\n"));
  process.exit(1);
}
console.log(`Validated ${contracts.length} contracts and bilingual navigation.`);
