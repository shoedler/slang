import DifferentName from "C:/Projects/slang/test/module/a" // [ExpectError] Uncaught error: Could not import module 'DifferentName'. File 'C:/Projects/slang/test/module/a' does not exist.
                                                            // [ExpectError]      1 | import DifferentName from "C:/Projects/slang/test/module/a"
                                                            // [ExpectError]          ~~~~~~~~~~~~~~~~~~~~~~~~~~
                                                            // [ExpectError]   at line 1 at the toplevel of module "main"