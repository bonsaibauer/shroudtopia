const languageFromPath = (): "en" | "de" =>
  location.pathname.split("/").includes("de") ? "de" : "en";

const switchLanguage = (target: "en" | "de"): void => {
  localStorage.setItem("shroudtopia-language", target);
  const source = languageFromPath();
  const segments = location.pathname.split("/");
  const index = segments.lastIndexOf(source);
  if (index >= 0) segments[index] = target;
  else segments.push(target, "");
  location.assign(`${segments.join("/")}${location.search}${location.hash}`);
};

const createLanguageSwitch = (): void => {
  const existing = document.querySelector<HTMLElement>(".right-buttons");
  const host = existing ?? document.querySelector<HTMLElement>(".menu-bar");
  if (!host || document.querySelector(".language-switch")) return;
  const current = languageFromPath();
  const nav = document.createElement("nav");
  nav.className = "language-switch";
  nav.setAttribute("aria-label", current === "de" ? "Sprache wählen" : "Choose language");
  for (const language of ["en", "de"] as const) {
    const button = document.createElement("button");
    button.type = "button";
    button.textContent = language === "en" ? "🇬🇧" : "🇩🇪";
    button.title = language === "en" ? "English" : "Deutsch";
    button.setAttribute("aria-label", button.title);
    button.setAttribute("aria-pressed", String(language === current));
    button.addEventListener("click", () => switchLanguage(language));
    nav.append(button);
  }
  host.prepend(nav);
};

const addReferenceFilter = (): void => {
  const functions = [...document.querySelectorAll<HTMLElement>(".api-function")];
  if (!functions.length) return;
  const input = document.createElement("input");
  input.className = "api-filter";
  input.type = "search";
  input.placeholder = languageFromPath() === "de" ? "Funktionen filtern …" : "Filter functions …";
  input.setAttribute("aria-label", input.placeholder);
  const select = document.createElement("select");
  select.className = "api-status-filter";
  const options = languageFromPath() === "de"
    ? [["", "Alle Status"], ["stable", "Stabil"], ["experimental", "Experimentell"]]
    : [["", "All statuses"], ["stable", "Stable"], ["experimental", "Experimental"]];
  for (const [value, label] of options) select.add(new Option(label, value));
  const apply = (): void => {
    const query = input.value.trim().toLowerCase();
    for (const fn of functions) fn.hidden = !fn.dataset.apiName?.includes(query) ||
      (!!select.value && fn.dataset.apiStatus !== select.value);
  };
  input.addEventListener("input", apply);
  select.addEventListener("change", apply);
  const controls = document.createElement("div");
  controls.className = "api-filter-bar";
  controls.append(input, select);
  functions[0].before(controls);
};

const addCopyFeedback = (): void => {
  for (const button of document.querySelectorAll<HTMLButtonElement>(".clip-button")) {
    button.addEventListener("click", () => {
      const original = button.title;
      button.title = languageFromPath() === "de" ? "Kopiert" : "Copied";
      window.setTimeout(() => { button.title = original; }, 1200);
    });
  }
};

document.addEventListener("DOMContentLoaded", () => {
  document.documentElement.lang = languageFromPath();
  createLanguageSwitch();
  addReferenceFilter();
  addCopyFeedback();
});
