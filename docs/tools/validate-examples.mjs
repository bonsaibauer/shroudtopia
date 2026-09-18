import { access, readFile } from "node:fs/promises";
import path from "node:path";
import process from "node:process";

const root = process.cwd();
const example = path.join(root, "docs", "api", "examples", "reference.cpp");
await access(example);
const source = await readFile(example, "utf8");
for (const required of ["shroudtopia.h", "example_register_command", "example_read_asset", "RESULT_OK"]) if (!source.includes(required)) { console.error(`docs/api/examples/reference.cpp: missing ${required}`); process.exit(1); }
if (source.includes("...")) { console.error("docs/api/examples/reference.cpp: examples must not contain ellipsis placeholders"); process.exit(1); }
console.log("Validated compile-targeted API examples.");
