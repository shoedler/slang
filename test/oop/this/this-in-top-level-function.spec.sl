fn foo() { // [exit] 2
  this // [expect-error] Resolver error at line 2: Can't use 'this' outside of a class.
}      // [expect-error]      2 |   this
       // [expect-error]            ~~~~
       // [expect-error] Resolver error at line 2: Undefined variable 'this'.
       // [expect-error]      2 |   this
       // [expect-error]            ~~~~