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
    const languageName = displayNames.of(language) || language;
    const button = element("button", "", `${language.toUpperCase()} · ${languageName}`);
    button.type = "button";
    button.lang = language;
    button.title = languageName;
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
document.addEventListener("DOMContentLoaded", async () => {
  await loadLanguages();
  document.documentElement.lang = languageFromPath();
  const ui = await loadUi();
  createLanguageSwitch(ui); enhanceReference(ui);
});
