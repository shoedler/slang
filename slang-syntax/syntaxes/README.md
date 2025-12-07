# SlangScript Neovim/Vim Syntax Highlighting

This directory contains syntax highlighting files for SlangScript.

## Available Files

- **slang.tmLanguage.json** - TextMate grammar for VSCode and other editors
- **slangscript.vim** - Vim/Neovim syntax file for simple syntax highlighting

## Installation (Neovim/Vim)

> **Note:** The filetype is set to `slangscript` to avoid conflicts with Vim's built-in S-Lang syntax file.

### Option 1: Manual Installation

1. Copy `slangscript.vim` to your Neovim/Vim syntax directory:
   ```bash
   # For Neovim
   mkdir -p ~/.config/nvim/syntax
   cp slangscript.vim ~/.config/nvim/syntax/
   
   # For Vim
   mkdir -p ~/.vim/syntax
   cp slangscript.vim ~/.vim/syntax/
   ```

2. Add filetype detection to your config:
   ```vim
   " For init.vim or .vimrc
   au BufRead,BufNewFile *.sl set filetype=slangscript
   ```
   
   Or for Neovim with Lua (init.lua):
   ```lua
   vim.cmd([[
     au BufRead,BufNewFile *.sl set filetype=slangscript
   ]])
   ```

### Option 2: Using lazy.nvim (Recommended for Neovim)

Add to your lazy.nvim config:

```lua
{
  dir = "/path/to/slang/slang-syntax",
  lazy = false,
  config = function()
    vim.cmd([[
      au BufRead,BufNewFile *.sl set filetype=slangscript
    ]])
  end,
}
```

### Option 3: Using packer.nvim

```lua
use {
  '/path/to/slang/slang-syntax',
  config = function()
    vim.cmd([[
      au BufRead,BufNewFile *.sl set filetype=slangscript
    ]])
  end,
}
```

### Option 4: Using vim-plug

Add to your .vimrc or init.vim:

```vim
Plug '/path/to/slang/slang-syntax'
au BufRead,BufNewFile *.sl set filetype=slangscript
```

### Option 5: Adding to runtimepath directly

In your init.vim or .vimrc:

```vim
set runtimepath+=path/to/slang/slang-syntax
au BufRead,BufNewFile *.sl set filetype=slangscript
```

Or in init.lua:

```lua
vim.opt.runtimepath:append('/path/to/slang/slang-syntax')
vim.cmd([[
  au BufRead,BufNewFile *.sl set filetype=slangscript
]])
```

## Maintenance

The `slangscript.vim` file is directly derived from `slang.tmLanguage.json`. If you update the tmGrammar file, you should update the vim syntax file accordingly.

### Mapping between tmGrammar and Vim Syntax

The vim syntax file follows the same structure as the tmGrammar:

| tmGrammar Section | Vim Syntax Equivalent |
|-------------------|----------------------|
| `comments` | `slangComment`, `slangCommentAnnotation` |
| `keywords` (control) | `slangControlFlow` |
| `keywords` (expression) | `slangExpressionOp` |
| `keywords` (logical) | `slangLogicalOp` |
| `keywords` (storage) | `slangStorageType`, `slangStorageKeyword` |
| `types` | `slangType` |
| `constants` | `slangConstant` |
| `numbers` | `slangNumber`, `slangFloat`, `slangHex`, etc. |
| `strings` | `slangString`, `slangEscape` |
| `variables` | `slangSpecialVar`, `slangVariable`, etc. |
| `operators` | `slangArithmeticOp`, `slangAssignmentOp`, etc. |

## Features

The vim syntax file provides:

- ✅ Keyword highlighting (if, for, while, let, const, etc.)
- ✅ Type highlighting (Obj, Nil, Bool, Num, Int, Float, Str, Seq, Tuple, Fn, Class)
- ✅ Comment highlighting with annotation support ([expect], [skip], etc.)
- ✅ Number literals (decimal, float, hex, binary, octal)
- ✅ String literals with escape sequences
- ✅ Function and class name highlighting
- ✅ Special variables (this, base, error, etc.)
- ✅ Operator highlighting
- ✅ Constant name detection (ALL_CAPS)
- ✅ Type name detection (PascalCase)

## Testing

To test the syntax highlighting:

1. Open a `.sl` file in Neovim/Vim
2. Verify that keywords, strings, numbers, and comments are highlighted
3. Check that the syntax follows your color scheme

Example test file (sample.sl is available in the repository root).

## Notes

- This is a simple syntax highlighting file, not a full LSP
- It works with any Neovim/Vim color scheme
- No external dependencies required
- Compatible with both Vim and Neovim
