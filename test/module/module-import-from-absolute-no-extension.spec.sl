import DifferentName from "C:/Projects/slang/test/module/a" // [expect-error] Uncaught error: Could not import module 'DifferentName'. File 'C:/Projects/slang/test/module/a' does not exist.
                                                            // [expect-error]      1 | import DifferentName from "C:/Projects/slang/test/module/a"
                                                            // [expect-error]          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                                            // [expect-error]   at line 1 at the toplevel of module "main"