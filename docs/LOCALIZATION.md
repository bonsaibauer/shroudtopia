# Documentation localization

English files under `docs/content/en` are the Crowdin source. Translations keep the same relative path under `docs/content/%two_letters_code%`; `_ui.json` contains the interface text used by the language selector and generated reference pages.

Configure Crowdin as a file-based project with the GitHub integration in **Source and translation files** mode. Connect branch `1.1.0`, use the repository-root `crowdin.yml`, import existing translations once, leave **Push Sources** disabled, and let Crowdin maintain its localization branch and pull request. Credentials and project IDs stay in Crowdin/GitHub and are never committed.

A language becomes available automatically when `docs/content/<locale>/SUMMARY.md` exists. The build discovers every such directory, emits `site/languages.json`, builds the book, and exposes a language-code button. `Intl.DisplayNames` supplies the readable language name, so flags, emoji, icon fonts, and code changes are unnecessary. English is the fallback.

Crowdin must preserve Markdown, fenced code, identifiers, capability IDs, paths, JSON keys, and relative links. CI compares every present locale with English, checks links and unchanged code blocks, validates `crowdin.yml`, and rejects credentials.
