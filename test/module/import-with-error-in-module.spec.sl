import f

f.fail() // [ExpectError] Uncaught error: Error!
         // [ExpectError]      2 |   throw "Error!"
         // [ExpectError]            ~~~~~~
         // [ExpectError]   at line 2 in "fail" in module "f"
         // [ExpectError]   at line 3 at the toplevel of module "main"