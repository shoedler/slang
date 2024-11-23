fn x {            // [exit] 2
  let {a} = {a:1} // [expect-error] Compile error at line 2 at 'a': Can't read local variable in its own initializer.
} // [expect-error] Compile error at line 3 at end: Expecting '}' after block.