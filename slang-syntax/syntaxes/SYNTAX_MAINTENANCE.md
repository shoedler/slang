# SlangScript Vim Syntax Quick Reference

This file provides a quick reference for maintaining the `slangscript.vim` syntax file.

## Syntax Group Mapping

The vim syntax file directly maps to the slang.tmLanguage.json structure:

### Comments
```vim
slangComment           -> slang.tmLanguage.json: comments
slangCommentAnnotation -> slang.tmLanguage.json: comments.patterns (for [expect], [skip], etc.)
```

### Keywords
```vim
slangControlFlow       -> slang.tmLanguage.json: keywords (control flow: if, for, while, etc.)
slangExpressionOp      -> slang.tmLanguage.json: keywords (expression operators: typeof, is, in, etc.)
slangLogicalOp         -> slang.tmLanguage.json: keywords (logical: and, or)
slangStorageType       -> slang.tmLanguage.json: keywords (storage: let, const, ctor)
slangStorageKeywordCls -> slang.tmLanguage.json: keywords (cls keyword)
slangStorageKeywordFn  -> slang.tmLanguage.json: keywords (fn keyword)
slangStorageModifier   -> slang.tmLanguage.json: keywords (modifiers: static)
```

### Types
```vim
slangType              -> slang.tmLanguage.json: types (Obj, Nil, Bool, Num, Int, Float, Str, Seq, Tuple, Fn, Class)
```

### Constants
```vim
slangConstant          -> slang.tmLanguage.json: constants (nil, true, false)
```

### Numbers
```vim
slangNumber            -> slang.tmLanguage.json: numbers (decimal integers)
slangFloat             -> slang.tmLanguage.json: numbers (floating point)
slangHex               -> slang.tmLanguage.json: numbers (hexadecimal: 0x...)
slangBinary            -> slang.tmLanguage.json: numbers (binary: 0b...)
slangOctal             -> slang.tmLanguage.json: numbers (octal: 0o...)
```

### Strings
```vim
slangString            -> slang.tmLanguage.json: strings
slangEscape            -> slang.tmLanguage.json: strings.patterns (escape sequences)
```

### Variables
```vim
slangSpecialVar        -> slang.tmLanguage.json: variables (this, base, error, etc.)
slangProperty          -> slang.tmLanguage.json: variables (property access: .len, .__name)
slangConstantName      -> slang.tmLanguage.json: variables (ALL_CAPS identifiers)
slangTypeName          -> slang.tmLanguage.json: variables (PascalCase identifiers)
slangFunctionCall      -> slang.tmLanguage.json: variables (function calls: name())
slangVariable          -> slang.tmLanguage.json: variables (generic identifiers)
slangClassName         -> slang.tmLanguage.json: keywords (class names after cls)
slangFunctionName      -> slang.tmLanguage.json: keywords (function names after fn)
```

### Operators
```vim
slangArithmeticOp      -> slang.tmLanguage.json: operators (arithmetic: +, -, *, /, %, ^)
slangAssignmentOp      -> slang.tmLanguage.json: operators (assignment: =, ->)
slangComparisonOp      -> slang.tmLanguage.json: operators (comparison: ==, !=, <, >, <=, >=)
slangLogicalSymbol     -> slang.tmLanguage.json: operators (logical symbols: !, ?, :)
```

## How to Update

1. **Adding a new keyword**: Add it to the appropriate `syntax keyword` line
2. **Adding a new type**: Add it to the `slangType` keyword list
3. **Adding a new constant**: Add it to the `slangConstant` keyword list
4. **Adding a new pattern**: Add a `syntax match` or `syntax region` in the appropriate section
5. **Changing colors**: Modify the `highlight default link` statements at the end

## Important Pattern Order

In Vim syntax files, order matters for `syntax match` patterns:
1. More specific patterns should come BEFORE general patterns
2. Keywords have higher priority than matches
3. Regions can contain other syntax groups

Current order (from specific to general):
1. Function calls (name followed by `(`)
2. ALL_CAPS constants
3. PascalCase type names
4. Generic variables (most general)

## Testing Changes

After making changes:
1. Copy the file: `cp syntaxes/slangscript.vim syntax/slangscript.vim`
2. Test with: `vim -u NONE -c 'set runtimepath+=.' -c 'syntax on' -c 'autocmd BufRead,BufNewFile *.sl set filetype=slangscript' sample.sl`
3. Check syntax groups at specific positions with: `:echo synIDattr(synID(line("."), col("."), 1), "name")`

## Vim Highlight Groups

The syntax file uses these standard Vim highlight groups:
- `Comment` - comments
- `SpecialComment` - special comment annotations
- `Conditional` - control flow keywords
- `Keyword` - expression operators
- `Operator` - operators and logical ops
- `StorageClass` - storage keywords (let, const, fn, cls, etc.)
- `Type` - types and type names
- `Boolean` - boolean constants
- `Number` - numeric literals
- `Float` - floating point literals
- `String` - string literals
- `SpecialChar` - escape sequences
- `Special` - special variables
- `Identifier` - identifiers and variables
- `Constant` - constants
- `Function` - function names and calls
