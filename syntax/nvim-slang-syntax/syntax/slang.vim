" Vim syntax file for SlangScript
" Language: Slang 
" Maintainer: Generated from slang.tmLanguage.json
" Latest Revision: 2025
" File Types: .sl
"
" Note: Uses 'slangscript' filetype to avoid conflicts with Vim's built-in
" S-Lang syntax file.
"
" Installation:
" 1. Copy this file to ~/.config/nvim/syntax/ (Neovim) or ~/.vim/syntax/ (Vim)
" 2. Add to your init.vim/init.lua or .vimrc:
"    au BufRead,BufNewFile *.sl set filetype=slangscript
" 3. Or use with a plugin manager like lazy.nvim, packer.nvim, etc.
" Quit when a syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

" Comments
" Comment annotations like [expect], [skip], [expect-error], [exit]
syntax match slangCommentAnnotation "\[expect\]" contained
syntax match slangCommentAnnotation "\[skip\]" contained
syntax match slangCommentAnnotation "\[expect-error\]" contained
syntax match slangCommentAnnotation "\[exit\]" contained
syntax region slangComment start="//" end="$" contains=slangCommentAnnotation

" Keywords - Control Flow
syntax keyword slangControlFlow if for else while ret print import from throw try catch skip break

" Keywords - Expression Operators
syntax keyword slangExpressionOp typeof is in not cwd clock log

" Keywords - Logical Operators
syntax keyword slangLogicalOp or and

" Keywords - Storage Types
syntax keyword slangStorageType let const ctor

" Keywords - Class and Function Definition
syntax keyword slangStorageKeywordCls cls nextgroup=slangClassName skipwhite
syntax keyword slangStorageKeywordFn fn nextgroup=slangFunctionName skipwhite
syntax match slangClassName "\<\w\+\>" contained
syntax match slangFunctionName "\<\w\+\>" contained

" Keywords - Modifiers
syntax keyword slangStorageModifier static

" Types
syntax keyword slangType Obj Nil Bool Num Int Float Str Seq Tuple Fn Class

" Constants
syntax keyword slangConstant nil true false

" Numbers
syntax match slangNumber "\<[0-9]\+\>"
syntax match slangFloat "\<[0-9]\+\.[0-9]\+\>"
syntax match slangHex "\<0x[0-9A-Fa-f]\+\>"
syntax match slangBinary "\<0b[01]\+\>"
syntax match slangOctal "\<0o[0-7]\+\>"

" Strings
syntax region slangString start='"' end='"' contains=slangEscape
syntax match slangEscape "\\u[0-9A-Fa-f]\{4\}" contained
syntax match slangEscape "\\x[0-9A-Fa-f]\{2\}" contained
syntax match slangEscape "\\[0-7]\{1,3\}" contained
syntax match slangEscape "\\[bfnrt\"'\\]" contained

" Variables - Language Special
syntax keyword slangSpecialVar this base error __file_path __module_name __name

" Variables - Property Access
syntax match slangProperty "\.\s*(\<len\>)"
syntax match slangProperty "\.\s*(\<__name\>)"

" Variables - Function Calls (before generic variable patterns)
syntax match slangFunctionCall "\<\w\+\>\s*("me=e-1

" Variables - Constants (ALL_CAPS) - must be before PascalCase
syntax match slangConstantName "\<[A-Z][A-Z0-9_]*\>"

" Variables - Type Names (PascalCase) - must be after ALL_CAPS
syntax match slangTypeName "\<[A-Z][a-z]\w*\>"

" Variables - Local Variables (most generic, should be last)
"syntax match slangVariable "\<[a-z_]\w*\>"

" Operators - Arithmetic
" Match arithmetic operators
" Note: Division (/) uses negative lookahead (/)\@! to match '/' only when NOT followed by another '/'
" This prevents matching '//' which is the comment start sequence
syntax match slangArithmeticOp "[+\-*%^]"
syntax match slangArithmeticOp "\v/(/)\@!"

" Operators - Assignment
syntax match slangAssignmentOp "="
syntax match slangAssignmentOp "->"

" Operators - Comparison
syntax match slangComparisonOp "=="
syntax match slangComparisonOp "!="
syntax match slangComparisonOp "<"
syntax match slangComparisonOp ">"
syntax match slangComparisonOp "<="
syntax match slangComparisonOp ">="

" Operators - Logical/Ternary
syntax match slangLogicalSymbol "!"
syntax match slangLogicalSymbol "?"
syntax match slangLogicalSymbol ":"

" Highlight Links
highlight default link slangComment Comment
highlight default link slangCommentAnnotation SpecialComment

highlight default link slangControlFlow Conditional
highlight default link slangExpressionOp Keyword
highlight default link slangLogicalOp Operator
highlight default link slangStorageType StorageClass
highlight default link slangStorageKeywordCls StorageClass
highlight default link slangStorageKeywordFn StorageClass
highlight default link slangStorageModifier StorageClass

highlight default link slangClassName Type
highlight default link slangFunctionName Function

highlight default link slangType Type

highlight default link slangConstant Boolean

highlight default link slangNumber Number
highlight default link slangFloat Float
highlight default link slangHex Number
highlight default link slangBinary Number
highlight default link slangOctal Number

highlight default link slangString String
highlight default link slangEscape SpecialChar

highlight default link slangSpecialVar Special
highlight default link slangProperty Identifier
highlight default link slangConstantName Constant
highlight default link slangTypeName Type
highlight default link slangFunctionCall Function
highlight default link slangVariable Identifier

highlight default link slangArithmeticOp Operator
highlight default link slangAssignmentOp Operator
highlight default link slangComparisonOp Operator
highlight default link slangLogicalSymbol Operator

let b:current_syntax = "slang"
