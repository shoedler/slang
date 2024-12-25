// [exit] 2
fn foo(arg, arg) { // [expect-error] Resolver error at line 2: Parameter 'arg' is already declared.
  "body"           // [expect-error]      2 | fn foo(arg, arg) {
}                  // [expect-error]                      ~~~