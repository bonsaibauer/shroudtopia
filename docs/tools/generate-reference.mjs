import { mkdir, readFile, readdir, writeFile } from "node:fs/promises";
import path from "node:path";

const root = process.cwd();
const publicHeaderPath = path.join(root, "api", "shroudtopia.h");
const metadataDir = path.join(root, "docs", "api", "services");
const generatedRoot = path.join(root, "docs", "generated");

const members = {
  actions: { functions: ["register_action", "invoke_action", "get_action_state"], types: ["Action"], enums: ["ActionState"] },
  api: { functions: ["ShroudtopiaGetApi"], types: ["Api"] },
  assets: { functions: ["AssetVisitor", "list_assets", "get_asset", "update_asset", "create_asset", "reset_assets", "save_assets", "set_asset_field"], types: ["AssetId"] },
  capabilities: { functions: ["query_capability", "check_permission"], types: ["CapabilityInfo"] },
  commands: { functions: ["CommandCallback", "register_command", "invoke_command"], types: ["CommandDescriptor"] },
  core: { functions: [], types: ["StringView"], enums: ["Result"] },
  events: { functions: ["EventCallback", "subscribe_event", "publish_event"], types: ["Event", "EventSubscription"] },
  lifecycle: { functions: ["ModLifecycleCallback", "ModUpdateCallback", "CreateModFunction"], types: ["ModDescriptor"] },
  logging: { functions: ["log", "read_log_tail"], enums: ["LogLevel", "LogSource"] },
  "runtime-patches": { functions: ["create_patch", "set_patch_enabled", "get_patch_state", "release_patch"], types: ["RuntimeRelocation", "RuntimePatchOptions", "RuntimePatchState"], enums: ["RuntimePatchKind", "RuntimeRelocationKind"] },
  services: { functions: ["register_service", "find_service", "release_registration"], types: ["ServiceDescriptor", "ServiceRequest"] },
  settings: { functions: ["get_mod_setting_bool", "get_mod_setting_number"] },
  ui: { functions: ["create_text_window", "set_text_window_text", "get_text_window_status", "destroy_text_window"], types: ["TextWindowOptions"], enums: ["TextWindowStatus"] }
};

const normalize = (value) => value.replace(/\s+/g, " ").trim();
const humanize = (value) => value.replace(/_/g, " ").replace(/\b\w/g, (letter) => letter.toUpperCase());

const parameterDescriptions = {
  owner: "Stable identifier of the calling mod.", owner_id: "Stable identifier of the calling mod.",
  user_data: "Opaque context forwarded unchanged to the callback.", capacity: "Elements or bytes available in the output buffer.",
  required_size: "Receives the required buffer size.", required_count: "Receives the required element count.",
  registration: "Receives or identifies an ownership-bound registration.", input_json: "UTF-8 JSON input validated against the action schema.",
  value_json: "UTF-8 JSON representation of the new value.", json: "UTF-8 JSON representation of the resource.",
  buffer: "Caller-owned output buffer; may be null for a size query.", callback: "Callback invoked according to the operation contract."
};

const splitParameters = (source) => {
  const parameters = [];
  let current = "";
  let depth = 0;
  for (const character of source) {
    if (character === "(") depth += 1;
    if (character === ")") depth -= 1;
    if (character === "," && depth === 0) { parameters.push(normalize(current)); current = ""; }
    else current += character;
  }
  if (normalize(current) && normalize(current) !== "void") parameters.push(normalize(current));
  return parameters;
};

const parseParameter = (raw, index) => {
  const cleaned = normalize(raw);
  const match = cleaned.match(/([A-Za-z_]\w*)(?:\s*\[[^\]]*\])?$/);
  const hasName = match && cleaned.slice(0, match.index).trim().length > 0;
  const name = hasName ? match[1] : `argument_${index + 1}`;
  const type = hasName ? cleaned.slice(0, match.index).trim() : cleaned;
  const pointer = type.includes("*");
  const isConst = /\bconst\b/.test(type);
  const outputName = /^(api|out|output|result|required|written|value|allowed|state|status|information|registration|interface|entity|grid|patch|window|buffer)/.test(name);
  const direction = pointer && outputName && (!isConst || type.includes("**")) ? "out" : "in";
  const conditional = ["buffer", "entities", "values", "coverage"].includes(name);
  return {
    name, type, direction,
    required: conditional ? "conditional" : "yes",
    nullable: conditional ? "yes, for capacity query" : pointer ? "no" : "–",
    ownership: direction === "out" ? "caller-owned" : pointer || type === "StringView" ? "borrowed" : "value",
    description: parameterDescriptions[name] ?? `Value for ${humanize(name).toLowerCase()}.`
  };
};

const functionSummary = (name) => {
  const exact = {
    ShroudtopiaGetApi: "Negotiate the requested ABI version and return the process-wide host API.",
    GridRegionFromPoints: "Convert two inclusive world points into an aligned, half-open grid region.",
    register_service: "Publish a versioned service implementation owned by the calling mod.",
    find_service: "Resolve a service implementation by its exact contract version.",
    release_registration: "Release one registration handle.",
    query_capability: "Read availability and version information for a capability.",
    check_permission: "Check whether an owner may use a permission.", read_tail: "Read a bounded tail of an existing log file."
  };
  if (exact[name]) return exact[name];
  const verb = name.split("_")[0];
  const actions = { register: "Register", invoke: "Invoke", subscribe: "Subscribe to", publish: "Publish", get: "Read", set: "Set", list: "List", create: "Create", update: "Update", remove: "Remove", reset: "Reset", save: "Persist", release: "Release", destroy: "Destroy", stage: "Stage", log: "Write", find: "Find", check: "Check" };
  return `${actions[verb] ?? "Execute"} ${humanize(name.replace(new RegExp(`^${verb}_?`), "")).toLowerCase()}.`;
};

const parseHeader = (name, source) => {
  const compact = source.replace(/\/\*[\s\S]*?\*\//g, " ").replace(/\/\/[^\n]*/g, " ");
  const functions = [];
  const seen = new Set();
  const addFunction = (functionName, returnType, rawParameters, signature, kind) => {
    if (seen.has(`${kind}:${functionName}`)) return;
    seen.add(`${kind}:${functionName}`);
    functions.push({ name: functionName, returnType: normalize(returnType), signature: normalize(signature), kind, summary: functionSummary(functionName), parameters: splitParameters(rawParameters).map(parseParameter) });
  };
  for (const match of compact.matchAll(/([A-Za-z_]\w*)\s*\(CALL\*\s*([A-Za-z_]\w*)\)\s*\(([^;]*?)\)\s*;/gs)) addFunction(match[2], match[1], match[3], match[0], /^[A-Z]/.test(match[2]) ? "callback" : "method");
  for (const match of compact.matchAll(/API_EXPORT\s+([A-Za-z_]\w*)\s+CALL\s+([A-Za-z_]\w*)\s*\(([^;]*?)\)\s*;/gs)) addFunction(match[2], match[1], match[3], match[0], "export");
  for (const match of compact.matchAll(/static\s+inline\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*\(([^)]*)\)/gs)) addFunction(match[2], match[1], match[3], match[0], "inline utility");
  const structs = [...compact.matchAll(/typedef\s+struct\s+([A-Za-z_]\w*)\s*\{([\s\S]*?)\}\s*\1\s*;/g)].map((match) => ({ name: match[1], fields: match[2].split(";").map(normalize).filter((field) => field && !field.includes("(CALL*")).map((field) => { const fieldMatch = field.match(/([A-Za-z_]\w*)(?:\s*\[[^\]]*\])?$/); return { name: fieldMatch?.[1] ?? "field", type: fieldMatch ? field.slice(0, fieldMatch.index).trim() : field }; }) }));
  const enums = [...compact.matchAll(/typedef\s+enum\s+([A-Za-z_]\w*)\s*\{([\s\S]*?)\}\s*\1\s*;/g)].map((match) => ({ name: match[1], values: match[2].split(",").map(normalize).filter(Boolean).map((value) => { const [enumName, enumValue] = value.split("=").map(normalize); return { name: enumName, value: enumValue ?? "auto" }; }) }));
  const serviceId = source.match(/#define\s+\w+_SERVICE_ID\s+"([^"]+)"/)?.[1];
  const major = source.match(/#define\s+\w+_SERVICE_VERSION_MAJOR\s+(\d+)u/)?.[1];
  const minor = source.match(/#define\s+\w+_SERVICE_VERSION_MINOR\s+(\d+)u/)?.[1];
  return { name, source, functions, structs, enums, serviceId, version: major ? `${major}.${minor ?? "0"}` : undefined };
};

const resultRows = [
  ["RESULT_OK", "The operation completed successfully.", "Continue with the returned value."],
  ["RESULT_INVALID_ARGUMENT", "A pointer, structure size, value, or JSON document is invalid.", "Correct the call; do not retry unchanged."],
  ["RESULT_CONFLICT", "The requested ownership or mutation conflicts with existing state.", "Resolve the competing owner or operation."],
  ["RESULT_NOT_FOUND", "The requested resource or provider does not exist.", "Check identifiers and availability."],
  ["RESULT_VERSION_MISMATCH", "The requested API version does not match API 1.2.", "Request API 1.2."],
  ["RESULT_PERMISSION_DENIED", "The owner lacks the required permission.", "Declare and obtain the required permission."],
  ["RESULT_NOT_AVAILABLE", "The feature is unavailable in the current runtime state.", "Wait for the required state or degrade gracefully."],
  ["RESULT_CALLBACK_FAILED", "A consumer callback returned a failure.", "Inspect the callback and its user data."],
  ["RESULT_INTERNAL_ERROR", "The loader could not complete an otherwise valid operation.", "Log context and fail safely."]
];

const metadataFiles = (await readdir(metadataDir)).filter((name) => name.endsWith(".yml")).sort();
const services = [];
const canonical = parseHeader("shroudtopia.h", await readFile(publicHeaderPath, "utf8"));
for (const file of metadataFiles) {
  const metadata = JSON.parse(await readFile(path.join(metadataDir, file), "utf8"));
  const selection = members[metadata.slug];
  if (!selection) throw new Error(`No public-member mapping for ${metadata.slug}`);
  const headers = [{ ...canonical,
    functions: canonical.functions.filter((item) => selection.functions.includes(item.name)),
    structs: canonical.structs.filter((item) => (selection.types ?? []).includes(item.name)),
    enums: canonical.enums.filter((item) => (selection.enums ?? []).includes(item.name))
  }];
  services.push({ ...metadata, headers });
}

await mkdir(generatedRoot, { recursive: true });
const publicModel = { schemaVersion: 1, apiVersion: "1.2", generatedFrom: "api/shroudtopia.h", services: services.map(({ headers, header: _header, extra_headers: _extra, ...service }) => ({ ...service, headers: headers.map(({ source, ...header }) => header) })), results: resultRows.map(([name, meaning, handling]) => ({ name, meaning, handling })) };
await writeFile(path.join(generatedRoot, "api-model.json"), `${JSON.stringify(publicModel, null, 2)}\n`);
console.log(`Generated the static API model for ${services.length} contracts.`);
