# SlangScript Vim Syntax Quick Reference

This file provides a quick reference for maintaining the `slangscript.vim` syntax file.

## Syntax Group Mapping

The vim syntax file directly maps to the tmGrammar.json structure:

### Comments
```vim
slangComment           -> tmGrammar: comments
slangCommentAnnotation -> tmGrammar: comments.patterns (for [expect], [skip], etc.)
```

### Keywords
```vim
slangControlFlow       -> tmGrammar: keywords (control flow: if, for, while, etc.)
slangExpressionOp      -> tmGrammar: keywords (expression operators: typeof, is, in, etc.)
slangLogicalOp         -> tmGrammar: keywords (logical: and, or)
slangStorageType       -> tmGrammar: keywords (storage: let, const, ctor)
slangStorageKeywordCls -> tmGrammar: keywords (cls keyword)
slangStorageKeywordFn  -> tmGrammar: keywords (fn keyword)
slangStorageModifier   -> tmGrammar: keywords (modifiers: static)
```

### Types
```vim
slangType              -> tmGrammar: types (Obj, Nil, Bool, Num, Int, Float, Str, Seq, Tuple, Fn, Class)
```

### Constants
```vim
slangConstant          -> tmGrammar: constants (nil, true, false)
```

### Numbers
```vim
slangNumber            -> tmGrammar: numbers (decimal integers)
slangFloat             -> tmGrammar: numbers (floating point)
slangHex               -> tmGrammar: numbers (hexadecimal: 0x...)
slangBinary            -> tmGrammar: numbers (binary: 0b...)
slangOctal             -> tmGrammar: numbers (octal: 0o...)
```

### Strings
```vim
slangString            -> tmGrammar: strings
slangEscape            -> tmGrammar: strings.patterns (escape sequences)
```

### Variables
```vim
slangSpecialVar        -> tmGrammar: variables (this, base, error, etc.)
slangProperty          -> tmGrammar: variables (property access: .len, .__name)
slangConstantName      -> tmGrammar: variables (ALL_CAPS identifiers)
slangTypeName          -> tmGrammar: variables (PascalCase identifiers)
slangFunctionCall      -> tmGrammar: variables (function calls: name())
slangVariable          -> tmGrammar: variables (generic identifiers)
slangClassName         -> tmGrammar: keywords (class names after cls)
slangFunctionName      -> tmGrammar: keywords (function names after fn)
```

### Operators
```vim
slangArithmeticOp      -> tmGrammar: operators (arithmetic: +, -, *, /, %, ^)
slangAssignmentOp      -> tmGrammar: operators (assignment: =, ->)
slangComparisonOp      -> tmGrammar: operators (comparison: ==, !=, <, >, <=, >=)
slangLogicalSymbol     -> tmGrammar: operators (logical symbols: !, ?, :)
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
