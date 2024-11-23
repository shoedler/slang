"str".foo = "value" // [expect-error] Uncaught error: Type Str does not support property-set access.
                    // [expect-error]      1 | "str".foo = "value"
                    // [expect-error]                ~~~~~~~~~~~~~
                    // [expect-error]   at line 1 at the toplevel of module "main"