{
  let a = "value"
  let a = "other" // [ExpectError] Compile error at line 3 at 'a': Already a variable with this name in this scope.
} 
// [ExpectError] Compile error at line 6 at end: Expecting '}' after block.
