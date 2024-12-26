// [exit] 2
let a = "value"
let a = a  // [expect-error] Resolver error at line 3: Global variable 'a' is already declared.
print a    // [expect-error]      3 | let a = a
           // [expect-error]              ~
