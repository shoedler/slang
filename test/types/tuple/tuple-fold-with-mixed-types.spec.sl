// [exit] 3
// Passing an anonymous function with index
print (1,3,4).fold([], fn(acc, x) -> acc + x) // [expect-error] Uncaught error: Type Seq does not support "add".
                                              // [expect-error]      3 | print (1,3,4).fold([], fn(acc, x) -> acc + x)
                                              // [expect-error]                                                   ~~~
                                              // [expect-error]   at line 3 in "$anon_fn$" in module "main"
                                              // [expect-error]   at line 3 at the toplevel of module "main"
