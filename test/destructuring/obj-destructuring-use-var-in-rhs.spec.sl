let {a} = {a:1} // [ExpectError] Uncaught error: Undefined variable 'a'.
                // [ExpectError]      1 | let {a} = {a:1}
                // [ExpectError]               ~ (TODO: Manually added, fix this)
                // [ExpectError]   at line 1 at the toplevel of module "main"