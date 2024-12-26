// [exit] 2
// Tests that we correctly track the line info across multiline strings.
let a = "1
2
3
"

err // [expect-error] Resolver error at line 8: Undefined variable 'err'.
    // [expect-error]      8 | err
    // [expect-error]          ~~~