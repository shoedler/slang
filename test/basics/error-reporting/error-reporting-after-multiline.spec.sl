// Tests that we correctly track the line info across multiline strings.
let a = "1
2
3
"

err // [expect-error] Uncaught error: Undefined variable 'err'.
    // [expect-error]      7 | err
    // [expect-error]          ~~~
    // [expect-error]   at line 7 at the toplevel of module "main"