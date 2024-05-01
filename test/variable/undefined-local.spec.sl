{
  print notDefined  // [ExpectError] Uncaught error: Undefined variable 'notDefined'.
}                   // [ExpectError]      2 |   print notDefined
                    // [ExpectError]                  ~~~~~~~~~~ (TODO: Manually added this. Fix it)
                    // [ExpectError]   at line 2 at the toplevel of module "main"