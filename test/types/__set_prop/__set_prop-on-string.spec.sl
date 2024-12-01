// [exit] 3
"str".foo = "value" // [expect-error] Uncaught error: Type Str does not support property-set access.
                    // [expect-error]      2 | "str".foo = "value"
                    // [expect-error]                ~~~~~~~~~~~~~
                    // [expect-error]   at line 2 at the toplevel of module "main"