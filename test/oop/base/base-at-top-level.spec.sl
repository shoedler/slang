// [exit] 2
base.foo("bar") // [expect-error] Resolver error at line 2: Can't use 'base' outside of a class.
                // [expect-error]      2 | base.foo("bar")
                // [expect-error]          ~~~~

base.foo        // [expect-error] Resolver error at line 6: Can't use 'base' outside of a class.
                // [expect-error]      6 | base.foo
                // [expect-error]          ~~~~
                // [expect-error] Resolver error at line 2: Undefined variable 'this'.
                // [expect-error]      2 | base.foo("bar")
                // [expect-error]          ~~~~