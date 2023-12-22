{
  let a = "value"
  let a = "other" // [ExpectCompileError] ERROR at [line 3] at 'a': Already a variable with this name in this scope.
} 
// [ExpectCompileError] ERROR at [line 6] at end: Expecting '}' after block.
