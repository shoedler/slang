let a = [1,2,3,"hi"]

// Non-integer
let sqrt_of_3 = 1.73205
let almost_3 = sqrt_of_3 * sqrt_of_3
print almost_3                           // [expect] 2.9999972025
print try (a[almost_3] = "?") else error // [expect] Type Seq does not support set-subscripting with Float. Expected Int.
print a[3]                               // [expect] hi

// String
print try (a["3"] = "?") else error      // [expect] Type Seq does not support set-subscripting with Str. Expected Int.

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in __get_prop, __set_prop, __get_subs and __set_subs.
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"
