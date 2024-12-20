// This is a regression test. When closing upvalues for discarded locals, it
// wouldn't make sure it discarded the upvalue for the correct stack slot.
//
// Here we create two locals that can be closed over, but only the first one
// actually is. When "b" goes out of scope, we need to make sure we don't
// prematurely close "a".
let closure

{
  let a = "a"

  {
    let b = "b"
    fn return_a -> a

    closure = return_a

    if false {
      fn return_b -> b
    }
  }

  print closure() // [expect] a
}
