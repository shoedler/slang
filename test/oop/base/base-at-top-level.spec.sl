// [exit] 2
base.foo("bar") // [expect-error] Resolver error at line 2: Can't use 'base' outside of a class.
base.foo        // [expect-error]      2 | base.foo("bar")
                // [expect-error]          ~~~~
                // [expect-error] Resolver error at line 2: Undefined variable 'this'.
                // [expect-error]      2 | base.foo("bar")
                // [expect-error]          ~~~~
                // [expect-error] Resolver error at line 2: Undefined variable 'base'.
                // [expect-error]      2 | base.foo("bar")
                // [expect-error]          ~~~~
                // [expect-error] Resolver error at line 3: Can't use 'base' outside of a class.
                // [expect-error]      3 | base.foo
                // [expect-error]          ~~~~
                // [expect-error] Resolver error at line 3: Undefined variable 'this'.
                // [expect-error]      3 | base.foo
                // [expect-error]          ~~~~
                // [expect-error] Resolver error at line 3: Undefined variable 'base'.
                // [expect-error]      3 | base.foo
                // [expect-error]          ~~~~