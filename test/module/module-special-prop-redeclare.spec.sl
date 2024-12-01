// [exit] 3
const __module_name = "new_module" // [expect-error] Uncaught error: Variable '__module_name' is already defined.
                                   // [expect-error]      2 | const __module_name = "new_module"
                                   // [expect-error]                                ~~~~~~~~~~~~
                                   // [expect-error]   at line 2 in "main" in module "new_module"