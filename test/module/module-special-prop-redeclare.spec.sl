const __module_name = "new_module" // [expect-error] Uncaught error: Variable '__module_name' is already defined.
                                   // [expect-error]      1 | const __module_name = "new_module"
                                   // [expect-error]                                ~~~~~~~~~~~~
                                   // [expect-error]   at line 1 in "main" in module "new_module"