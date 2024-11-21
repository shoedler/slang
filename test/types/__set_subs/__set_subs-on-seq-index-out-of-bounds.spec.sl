let a = [1,2,3]

print a[4] = 1 // [ExpectError] Uncaught error: Index out of bounds. Was 4, but this Seq has length 3.
               // [ExpectError]      3 | print a[4] = 1
               // [ExpectError]                 ~~~~~~~
               // [ExpectError]   at line 3 at the toplevel of module "main"

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in __get_prop, __set_prop, __get_subs and __set_subs.
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"