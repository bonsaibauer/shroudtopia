import { createServer } from "node:http";
import { readFile, stat } from "node:fs/promises";
import { spawn } from "node:child_process";
import path from "node:path";
import readline from "node:readline";

const root = process.cwd();
const siteRoot = path.resolve(root, "site");
const model = JSON.parse(await readFile(path.join(root, "docs", "generated", "api-model.json"), "utf8"));
const option = (name, fallback) => {
  const index = process.argv.indexOf(name);
  return index >= 0 ? process.argv[index + 1] : fallback;
};
const port = Number(option("--port", "43117"));
const gameDirectory = option("--game-dir");
const stem = option("--stem", "enshrouded");
const allowWrite = process.argv.includes("--allow-write");
if (!Number.isInteger(port) || port < 1024 || port > 65535) throw new Error("--port must be between 1024 and 65535");
if (!gameDirectory) throw new Error('--game-dir is required, for example: npm run docs:serve -- --game-dir "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Enshrouded"');

const executable = path.join(root, "src", "assets", "target", "release", process.platform === "win32" ? "shroudtopia-inspect.exe" : "shroudtopia-inspect");
const childArguments = ["--game-dir", path.resolve(gameDirectory), "--stem", stem];
if (allowWrite) childArguments.push("--allow-write");
const inspector = spawn(executable, childArguments, { stdio: ["pipe", "pipe", "pipe"], windowsHide: true });
let inspectorError = "";
inspector.stderr.setEncoding("utf8");
inspector.stderr.on("data", (chunk) => { inspectorError += chunk; process.stderr.write(chunk); });
const lines = readline.createInterface({ input: inspector.stdout });
const pending = [];
lines.on("line", (line) => {
  const next = pending.shift();
  if (!next) return;
  try { next.resolve(JSON.parse(line)); } catch (error) { next.reject(error); }
});
inspector.on("error", (error) => { while (pending.length) pending.shift().reject(error); });
inspector.on("exit", (code) => {
  const error = new Error(`Asset inspector stopped with exit code ${code}.${inspectorError ? ` ${inspectorError.trim()}` : ""}`);
  while (pending.length) pending.shift().reject(error);
});
const invoke = (request) => new Promise((resolve, reject) => {
  if (inspector.exitCode !== null) return reject(new Error(inspectorError.trim() || "Asset inspector is not running"));
  pending.push({ resolve, reject });
  inspector.stdin.write(`${JSON.stringify(request)}\n`, (error) => { if (error) reject(error); });
});

const json = (response, status, body) => {
  const payload = Buffer.from(JSON.stringify(body, null, 2));
  response.writeHead(status, { "Content-Type": "application/json; charset=utf-8", "Content-Length": payload.length, "Cache-Control": "no-store" });
  response.end(payload);
};
const readBody = async (request) => {
  const chunks = [];
  for await (const chunk of request) chunks.push(chunk);
  return JSON.parse(Buffer.concat(chunks).toString("utf8"));
};
const validateInvocation = (invocation) => {
  if (!invocation || typeof invocation !== "object" || Array.isArray(invocation)) throw new Error("Invocation must be an object");
  const service = model.services.find((item) => item.slug === invocation.service);
  if (!service) throw new Error("Unknown service");
  const fn = service.headers.flatMap((header) => header.functions).find((item) => item.kind !== "callback" && item.name === invocation.function);
  if (!fn) throw new Error("Unknown function");
  if (!invocation.arguments || typeof invocation.arguments !== "object" || Array.isArray(invocation.arguments)) throw new Error("arguments must be an object");
  for (const parameter of fn.parameters.filter((item) => item.direction === "in" && item.required === "yes")) if (!(parameter.name in invocation.arguments)) throw new Error(`Missing ${parameter.name}`);
  return { service, fn };
};
const mime = { ".html": "text/html; charset=utf-8", ".css": "text/css; charset=utf-8", ".js": "text/javascript; charset=utf-8", ".json": "application/json; charset=utf-8", ".svg": "image/svg+xml", ".png": "image/png", ".woff2": "font/woff2" };

const server = createServer(async (request, response) => {
  try {
    const url = new URL(request.url ?? "/", "http://127.0.0.1");
    if (request.method === "POST" && ["/api/validate", "/api/sandbox"].includes(url.pathname)) {
      const invocation = await readBody(request);
      validateInvocation(invocation);
      if (url.pathname === "/api/validate") return json(response, 200, { result: "RESULT_OK" });
      const result = await invoke(invocation);
      return json(response, 200, { ...result, mode: "local", write_enabled: allowWrite });
    }
    if (request.method !== "GET" && request.method !== "HEAD") return json(response, 405, { error: "Method not allowed" });
    const requested = url.pathname === "/" ? "/index.html" : decodeURIComponent(url.pathname);
    let file = path.resolve(siteRoot, `.${requested}`);
    if (!file.startsWith(`${siteRoot}${path.sep}`) && file !== siteRoot) return json(response, 403, { error: "Forbidden" });
    try { if ((await stat(file)).isDirectory()) file = path.join(file, "index.html"); } catch { /* handled below */ }
    const body = await readFile(file);
    response.writeHead(200, { "Content-Type": mime[path.extname(file)] ?? "application/octet-stream", "Content-Length": body.length, "X-Content-Type-Options": "nosniff", "Cache-Control": "no-store" });
    if (request.method === "HEAD") response.end(); else response.end(body);
  } catch (error) {
    const notFound = error && typeof error === "object" && "code" in error && error.code === "ENOENT";
    json(response, notFound ? 404 : 400, { error: notFound ? "Not found" : error.message });
  }
});

const shutdown = () => { inspector.stdin.end(); server.close(); };
process.on("SIGINT", shutdown);
process.on("SIGTERM", shutdown);
server.listen(port, "127.0.0.1", () => {
  console.log(`Shroudtopia API Explorer: http://127.0.0.1:${port}/`);
  console.log(`Asset source: ${path.resolve(gameDirectory)} (${stem}); writes ${allowWrite ? "enabled" : "disabled"}.`);
});
