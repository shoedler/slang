// [exit] 2
if (false) {
  print not_defined // [expect-error] Resolver error at line 3: Undefined variable 'not_defined'.
}                   // [expect-error]      3 |   print not_defined
                    // [expect-error]                  ~~~~~~~~~~~

print "ok" 