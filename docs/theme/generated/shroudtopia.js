"use strict";
const languageFromPath = () => location.pathname.split("/").includes("de") ? "de" : "en";
const switchLanguage = (target) => {
    localStorage.setItem("shroudtopia-language", target);
    const source = languageFromPath();
    const segments = location.pathname.split("/");
    const index = segments.lastIndexOf(source);
    if (index >= 0)
        segments[index] = target;
    else
        segments.push(target, "");
    location.assign(`${segments.join("/")}${location.search}${location.hash}`);
};
const createLanguageSwitch = () => {
    const existing = document.querySelector(".right-buttons");
    const host = existing ?? document.querySelector(".menu-bar");
    if (!host || document.querySelector(".language-switch"))
        return;
    const current = languageFromPath();
    const nav = document.createElement("nav");
    nav.className = "language-switch";
    nav.setAttribute("aria-label", current === "de" ? "Sprache wählen" : "Choose language");
    for (const language of ["en", "de"]) {
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
const addReferenceFilter = () => {
    const functions = [...document.querySelectorAll(".api-function")];
    if (!functions.length)
        return;
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
    for (const [value, label] of options)
        select.add(new Option(label, value));
    const apply = () => {
        const query = input.value.trim().toLowerCase();
        for (const fn of functions)
            fn.hidden = !fn.dataset.apiName?.includes(query) ||
                (!!select.value && fn.dataset.apiStatus !== select.value);
    };
    input.addEventListener("input", apply);
    select.addEventListener("change", apply);
    const controls = document.createElement("div");
    controls.className = "api-filter-bar";
    controls.append(input, select);
    functions[0].before(controls);
};
const addCopyFeedback = () => {
    for (const button of document.querySelectorAll(".clip-button")) {
        button.addEventListener("click", () => {
            const original = button.title;
            button.title = languageFromPath() === "de" ? "Kopiert" : "Copied";
            window.setTimeout(() => { button.title = original; }, 1200);
        });
    }
};
const enhanceTables = () => {
    for (const table of document.querySelectorAll("main table")) {
        const headers = [...table.querySelectorAll("thead th")]
            .map((header) => header.textContent?.trim() ?? "");
        for (const row of table.querySelectorAll("tbody tr")) {
            [...row.cells].forEach((cell, index) => cell.dataset.label = headers[index] ?? "");
        }
        const frame = document.createElement("div");
        frame.className = "table-frame";
        frame.tabIndex = 0;
        frame.setAttribute("role", "region");
        frame.setAttribute("aria-label", languageFromPath() === "de" ? "Datentabelle" : "Data table");
        table.before(frame);
        frame.append(table);
    }
};
const markExternalLinks = () => {
    for (const link of document.querySelectorAll("main a[href^='http']")) {
        if (link.hostname === location.hostname)
            continue;
        link.target = "_blank";
        link.rel = "noopener noreferrer";
        link.classList.add("external-link");
    }
};
document.addEventListener("DOMContentLoaded", () => {
    document.documentElement.lang = languageFromPath();
    createLanguageSwitch();
    addReferenceFilter();
    addCopyFeedback();
    enhanceTables();
    markExternalLinks();
});
