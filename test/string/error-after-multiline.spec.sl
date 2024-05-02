// Tests that we correctly track the line info across multiline strings.
let a = "1
2
3
"

err // [ExpectError] Uncaught error: Undefined variable 'err'.
    // [ExpectError]      7 | err
    // [ExpectError]          ~~~
    // [ExpectError]   at line 7 at the toplevel of module "main"