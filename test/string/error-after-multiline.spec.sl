// Tests that we correctly track the line info across multiline strings.
let a = "1
2
3
"

err // [ExpectRuntimeError] Undefined variable 'err'.
    // [ExpectRuntimeError] [line 7] in fn toplevel