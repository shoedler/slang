{
  let a = "value" // [exit] 2
  let a = "other" // [expect-error] Compile error at line 3 at 'a': Already a variable with this name in this scope.
} 
// [expect-error] Compile error at line 6 at end: Expecting '}' after block.
