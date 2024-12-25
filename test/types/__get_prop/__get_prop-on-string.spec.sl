// [exit] 3
"str".foo // [expect-error] Uncaught error: Property 'foo' does not exist on value of type Str.
          // [expect-error]      2 | "str".foo
          // [expect-error]               ~~~~
          // [expect-error]   at line 2 at the toplevel of module "main"