let a = [1,2,3]
print a[0] // [Expect] 1

// Negative index should be allowed
print a[-1] // [Expect] 3

// Out of bound index should return nil
print a[3] // [Expect] nil
print a[-4] // [Expect] nil

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in value_get_property, value_set_property, value_get_index and value_set_index
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"