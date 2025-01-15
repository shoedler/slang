{
  let a = "value" // [exit] 2
  let a = "other" // [expect-error] Resolver error at line 3: Local variable 'a' is already declared.
}                 // [expect-error]      3 |   let a = "other"
                  // [expect-error]                ~