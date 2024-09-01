# curly-colors

A Work-in-progress low-level implementation for syntax-highlighting using `*.tmLanguage` and `*.sublime-syntax` grammar, implemented mostly in somewhat posix compliant c, and depends on `libyaml`, `libxml2` and `libonig` (oniguruma RegExp engine)

### Motivation

TextMate's syntax highlighting grammar have more or less become the defacto standard for how to express grammar for highlighting, more or less the grammar already exists for any language or markup format imaginable. Yet there is not that many libraries that provide the functionality of evaluating the grammar in a language which is portable.

The aim of this project is to provide a component that can be used as the syntax highlighting engine within a codemirror like component implemented nativly.


### Usage

Currently the whole functionallity is in testing/development phase and there is a utility to test and run parsing of grammar trough a tool executable trough command-line:

```bash
yaml-test -s ./sample/base16-one-light.tmTheme -f ./sample/sample.js
```

Later down the road when the implementation has been stabalized the plan is to provide the API trough a shared library.

### Links and attribution

@trishume's [syntect](https://github.com/trishume/syntect) have been used a reference for how `*.sublime-syntax` works, even if Rust land is not anything that I enjoy to visiting.

[TextMates grammar specification](https://macromates.com/manual/en/language_grammars)

[Documentation on the syntax definitions grammar used by Sublime](https://www.sublimetext.com/docs/syntax.html)

[Sublime's color scheme specification](https://www.sublimetext.com/docs/color_schemes.html)

[Sublime's documentation on the `*.tmtheme` files](https://www.sublimetext.com/docs/color_schemes_tmtheme.html)