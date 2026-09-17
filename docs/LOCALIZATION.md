# Documentation localization

Every page is ordinary Markdown in `docs/src`. The language is the final part of the filename:

- `status_en.md` is the English source.
- `status_de.md` is the German translation.

Both files stay beside each other and use the same relative links. `SUMMARY_en.md` and `SUMMARY_de.md` define the two navigations. The build script copies one language at a time into a temporary mdBook source directory; authors never edit that generated directory.

Crowdin reads every `*_en.md` file and writes the German `*_de.md` file beside it according to `/crowdin.yml`. Markdown syntax, links, and code blocks remain Markdown and must not be recreated in TypeScript. TypeScript only enhances the rendered HTML.
