fn x {
  let [a] = [a] // [ExpectError] Compile error at line 2 at 'a': Can't read local variable in its own initializer.
} // [ExpectError] Compile error at line 3 at end: Expecting '}' after block.