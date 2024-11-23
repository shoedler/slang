cls A {
  fn x -> 1
  static fn y -> 2
}

let a = A()

// Cannot access methods / static methods via get-subscripting.
print a["x"]                // [expect] nil
print try A["y"] else error // [expect] Type Class does not support get-subscripting.

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in __get_prop, __set_prop, __get_subs and __set_subs.
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"
