# Contributing to the documentation

For every public API change:

1. update headers, implementation, and behavior tests;
2. update the matching contract in `docs/contracts` with fields, parameters, ownership, lifetime, threading, results, version, and example;
3. update English and German concepts/guides when semantics change;
4. run `npm run docs:check`;
5. review desktop/mobile and light/dark rendering for visible changes.

Generated pages contain a warning and must not be edited by hand. English and German `SUMMARY.md` paths must remain identical.
