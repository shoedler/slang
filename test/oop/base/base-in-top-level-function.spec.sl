// [exit] 2
  base.bar() // [expect-error] Resolver error at line 2: Can't use 'base' outside of a class.
fn foo() {   // [expect-error]      2 |   base.bar()
}            // [expect-error]            ~~~~