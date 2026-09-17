import { access, readFile } from "node:fs/promises";
import path from "node:path";

const root = process.cwd();
const model = JSON.parse(await readFile(path.join(root, "docs", "generated", "api-model.json"), "utf8"));
if (!Array.isArray(model.services) || model.services.length === 0) throw new Error("API Explorer model contains no services");
await access(path.join(root, "docs", "theme", "shroudtopia.js"));
const script = await readFile(path.join(root, "docs", "theme", "shroudtopia.js"), "utf8");
if (!script.includes("api-explorer")) throw new Error("Compiled theme does not contain the API Explorer");
if (/https?:\/\/(?:cdn|unpkg|jsdelivr)/.test(script)) throw new Error("API Explorer must not load unpinned CDN code");
console.log(`Built API Explorer contract for ${model.services.length} services.`);
