# Collett Translations

The translation files are `collett_<lang>_<country>.ts`, like `collett_nb_NO.ts`. The source
text is always British English, so other variants of English, like `en_US`, are translations too.

`collett_base.ts` holds all the source text, and is the file for the translation platform. It is
not built into the app.

## Updating

Create or update translation files from the source code with:

```bash
./pkgutils.py qtlupdate i18n/collett_base.ts i18n/collett_<lang>_<country>.ts
```

Edit the file with Qt Linguist. The `.qm` files are built with the app, so rebuild and set
`Main/guiLanguage` in the config file to try a translation.

The source text marks plurals with "(s)", like "%Ln word(s)". Translations give their own plural
forms, and the app picks the English form when there is no translation.

## Contexts

Terms used in many places are translated once, in the `Label` and `Stats` contexts.

* `Label` holds the names of things in a project, like "Chapter" and "Scene". "Characters" here
  means the people of a story.
* `Stats` holds text statistics. "Characters" here means letters, digits, spaces and symbols.
