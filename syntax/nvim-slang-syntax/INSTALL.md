# Install Neovim

Lazy:

```lua
{
    dir = '~/Projects/slang/syntax/nvim-slang-syntax',
    lazy = false,
    config = function()
      vim.api.nvim_create_autocmd({ 'BufRead', 'BufNewFile' }, {
        pattern = '*.sl',
        callback = function()
          vim.bo.filetype = 'slang'
        end,
      })
    end,
}
```

If you have Treesitter, then you'd want to add:

```lua
  { 
    'nvim-treesitter/nvim-treesitter',
    ignore_install = { 'slang' },
    ...
```

And, you might need to run `:TSUninstall slang`, if Treesitter auto-installed it for *.sl files. 
