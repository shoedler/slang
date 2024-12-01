// [exit] 3
{
  print not_defined  // [expect-error] Uncaught error: Undefined variable 'not_defined'.
}                   // [expect-error]      3 |   print not_defined
                    // [expect-error]                  ~~~~~~~~~~~
                    // [expect-error]   at line 3 at the toplevel of module "main"