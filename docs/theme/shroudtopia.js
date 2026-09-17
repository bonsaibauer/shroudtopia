"use strict";

const pathSegments = () => location.pathname.split("/").filter(Boolean);
let supportedLanguages = ["en"];
let fallbackLanguage = "en";
const languageFromPath = () => pathSegments().find(segment => supportedLanguages.includes(segment)) || fallbackLanguage;
const languageRoot = () => {
  const segments = location.pathname.split("/");
  const index = segments.findIndex(segment => supportedLanguages.includes(segment));
  return index >= 0 ? `${segments.slice(0, index + 1).join("/")}/` : "./";
};
const loadLanguages = async () => {
  const guessed = location.pathname.match(/\/([a-z]{2,3}(?:-[A-Za-z0-9]+)?)\//)?.[1] || "en";
  const guessedRoot = location.pathname.slice(0, Math.max(0, location.pathname.indexOf(`/${guessed}/`) + 1));
  try {
    const response = await fetch(`${guessedRoot}languages.json`);
    if (!response.ok) throw new Error();
    const data = await response.json();
    supportedLanguages = data.languages;
    fallbackLanguage = data.fallback;
  } catch { supportedLanguages = [guessed]; fallbackLanguage = guessed; }
};
const defaultUi = {
  chooseLanguage: "Choose language", filterFunctions: "Filter functions …", allStatuses: "All statuses",
  stable: "Stable", experimental: "Experimental", copied: "Copied", dataTable: "Data table",
  service: "Service", function: "Function", arguments: "Arguments (JSON)", validate: "Validate",
  execute: "Execute locally", valid: "Input is valid.", invalid: "Invalid input", result: "Complete result",
  localHelp: "Real execution requires the local host. Writes require --allow-write and save_assets.",
  code: "Generated C++ pattern", loadError: "Could not load API Explorer"
};
const loadUi = async () => {
  try {
    const response = await fetch(`${languageRoot()}assets/ui.json`);
    return response.ok ? { ...defaultUi, ...await response.json() } : defaultUi;
  } catch { return defaultUi; }
};
const switchLanguage = target => {
  localStorage.setItem("shroudtopia-language", target);
  const segments = location.pathname.split("/");
  const index = segments.findIndex(segment => supportedLanguages.includes(segment));
  if (index >= 0) segments[index] = target;
  location.assign(`${segments.join("/")}${location.search}${location.hash}`);
};
const element = (name, className, text) => {
  const node = document.createElement(name);
  if (className) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
};
const createLanguageSwitch = (ui) => {
  const host = document.querySelector(".right-buttons") || document.querySelector(".menu-bar");
  if (!host || document.querySelector(".language-switch")) return;
  const current = languageFromPath();
  const displayNames = new Intl.DisplayNames([current, fallbackLanguage], { type: "language" });
  const nav = element("nav", "language-switch");
  nav.setAttribute("aria-label", ui.chooseLanguage);
  for (const language of supportedLanguages) {
    const button = element("button", "", language.toUpperCase());
    button.type = "button";
    button.lang = language;
    button.title = displayNames.of(language) || language;
    button.setAttribute("aria-label", button.title);
    button.setAttribute("aria-pressed", String(language === current));
    button.addEventListener("click", () => switchLanguage(language));
    nav.append(button);
  }
  host.prepend(nav);
};
const enhanceReference = (ui) => {
  const functions = [...document.querySelectorAll(".api-function")];
  if (functions.length) {
    const input = element("input", "api-filter");
    input.type = "search"; input.placeholder = ui.filterFunctions; input.setAttribute("aria-label", ui.filterFunctions);
    const select = element("select", "api-status-filter");
    for (const [value, label] of [["", ui.allStatuses], ["stable", ui.stable], ["experimental", ui.experimental]]) select.add(new Option(label, value));
    const apply = () => { for (const fn of functions) fn.hidden = !fn.dataset.apiName?.includes(input.value.trim().toLowerCase()) || (!!select.value && fn.dataset.apiStatus !== select.value); };
    input.addEventListener("input", apply); select.addEventListener("change", apply);
    const controls = element("div", "api-filter-bar"); controls.append(input, select); functions[0].before(controls);
  }
  for (const button of document.querySelectorAll(".clip-button")) button.addEventListener("click", () => {
    const original = button.title; button.title = ui.copied; window.setTimeout(() => { button.title = original; }, 1200);
  });
  for (const table of document.querySelectorAll("main table")) {
    const headers = [...table.querySelectorAll("thead th")].map(header => header.textContent?.trim() || "");
    for (const row of table.querySelectorAll("tbody tr")) [...row.cells].forEach((cell, index) => { cell.dataset.label = headers[index] || ""; });
    const frame = element("div", "table-frame"); frame.tabIndex = 0; frame.role = "region"; frame.setAttribute("aria-label", ui.dataTable); table.before(frame); frame.append(table);
  }
  for (const link of document.querySelectorAll("main a[href^='http']")) if (link.hostname !== location.hostname) { link.target = "_blank"; link.rel = "noopener noreferrer"; link.classList.add("external-link"); }
};
const parameterTemplate = parameter => {
  if (parameter.direction === "out") return "<output>";
  if (parameter.name === "owner_id") return "sandbox";
  if (parameter.name === "type_name") return "<qualified asset type>";
  if (parameter.name === "json") return {};
  if (parameter.name === "path") return "/field";
  if (parameter.name === "asset") return { guid: "<guid>", type_name: "<qualified asset type>", part: 0 };
  return null;
};
const initializeApiExplorer = async (ui) => {
  const host = document.querySelector("#api-explorer");
  if (!host) return;
  try {
    const response = await fetch(`${languageRoot()}assets/api-model.json`);
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const model = await response.json();
    const navigation = element("aside", "api-explorer-navigation");
    const workspace = element("section", "api-explorer-workspace");
    const editor = element("aside", "api-explorer-editor");
    const serviceLabel = element("label", "", ui.service), serviceSelect = element("select"); serviceLabel.append(serviceSelect);
    const functionLabel = element("label", "", ui.function), functionSelect = element("select"); functionLabel.append(functionSelect);
    navigation.append(serviceLabel, functionLabel);
    const heading = element("h2"), summary = element("p"), signature = element("pre", "api-explorer-signature"), capabilities = element("ul", "api-explorer-capabilities");
    workspace.append(heading, summary, signature, capabilities);
    const argumentsLabel = element("label", "", ui.arguments), argumentsInput = element("textarea", "api-explorer-arguments"); argumentsInput.spellcheck = false; argumentsLabel.append(argumentsInput);
    const actions = element("div", "api-explorer-actions"), validate = element("button", "", ui.validate), execute = element("button", "", ui.execute);
    validate.type = execute.type = "button"; actions.append(validate, execute);
    const help = element("p", "api-explorer-help", ui.localHelp), output = element("output", "api-explorer-output"), codeTitle = element("h3", "", ui.code), code = element("pre", "api-explorer-code");
    editor.append(argumentsLabel, actions, help, output, codeTitle, code); host.replaceChildren(navigation, workspace, editor);
    const service = () => model.services[serviceSelect.selectedIndex];
    const functions = () => service().headers.flatMap(header => header.functions).filter(item => item.kind !== "callback");
    const fn = () => functions()[functionSelect.selectedIndex];
    const renderFunction = () => {
      const selected = fn();
      if (!selected) {
        heading.textContent = languageFromPath() === "de" ? service().title_de : service().title;
        summary.textContent = languageFromPath() === "de" ? service().summary_de : service().summary;
        signature.textContent = ""; capabilities.replaceChildren(); editor.hidden = true; return;
      }
      editor.hidden = false;
      heading.textContent = selected.name; summary.textContent = selected.summary; signature.textContent = selected.signature;
      capabilities.replaceChildren(...service().capabilities.map(item => element("li", "", item)));
      argumentsInput.value = JSON.stringify(Object.fromEntries(selected.parameters.filter(item => item.direction === "in").map(item => [item.name, parameterTemplate(item)])), null, 2);
      code.textContent = `Result result = api->${selected.name}(/* arguments */);\nif (result != RESULT_OK) {\n    api->log(owner_id, LOG_ERROR, {"API call failed", 15});\n}`;
      output.textContent = "";
    };
    const renderService = () => { functionSelect.replaceChildren(...functions().map(item => new Option(item.name, item.name))); renderFunction(); };
    const language = languageFromPath();
    serviceSelect.replaceChildren(...model.services.map(item => new Option(language === "de" ? item.title_de : item.title, item.slug)));
    serviceSelect.addEventListener("change", renderService); functionSelect.addEventListener("change", renderFunction);
    const parse = () => {
      const value = JSON.parse(argumentsInput.value);
      if (!value || Array.isArray(value) || typeof value !== "object") throw new Error("Arguments must be a JSON object");
      for (const parameter of fn().parameters.filter(item => item.direction === "in" && item.required === "yes")) if (!(parameter.name in value)) throw new Error(`Missing ${parameter.name}`);
      return value;
    };
    validate.addEventListener("click", () => { try { parse(); output.textContent = ui.valid; output.dataset.state = "success"; } catch (error) { output.textContent = `${ui.invalid}: ${error.message}`; output.dataset.state = "error"; } });
    execute.addEventListener("click", async () => {
      try {
        if (!["127.0.0.1", "localhost"].includes(location.hostname)) throw new Error(ui.localHelp);
        const request = { service: service().slug, function: fn().name, arguments: parse() };
        const local = await fetch("/api/sandbox", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(request) });
        const result = await local.json(); if (!local.ok) throw new Error(result.error || `HTTP ${local.status}`);
        output.textContent = `${ui.result}:\n${JSON.stringify(result, null, 2)}`; output.dataset.state = result.result === "RESULT_OK" ? "success" : "notice";
      } catch (error) { output.textContent = `${ui.invalid}: ${error.message}`; output.dataset.state = "error"; }
    });
    renderService();
  } catch (error) { host.textContent = `${ui.loadError}: ${error.message}`; }
};

document.addEventListener("DOMContentLoaded", async () => {
  await loadLanguages();
  document.documentElement.lang = languageFromPath();
  const ui = await loadUi();
  createLanguageSwitch(ui); enhanceReference(ui); await initializeApiExplorer(ui);
});
