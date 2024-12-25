fn foo(a) { // [exit] 2
  let a // [expect-error] Resolver error at line 2: Local variable 'a' is already declared.
}       // [expect-error]      2 |   let a
        // [expect-error]                ~