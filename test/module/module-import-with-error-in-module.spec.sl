import f

f.fail() // [expect-error] Uncaught error: Error!
         // [expect-error]      2 |   throw "Error!"
         // [expect-error]            ~~~~~~~~~~~~~~
         // [expect-error]   at line 2 in "fail" in module "f"
         // [expect-error]   at line 3 at the toplevel of module "main"