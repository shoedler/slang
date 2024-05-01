"str".foo = "value" // [ExpectError] Uncaught error: Type Str does not support property-set access.
                    // [ExpectError]      1 | "str".foo = "value"
                    // [ExpectError]                ~~~~~~
                    // [ExpectError]   at line 1 at the toplevel of module "main"