print "foo"[0] = "a" // [expect-error] Uncaught error: Type Str does not support set-subscripting.
                     // [expect-error]      1 | print "foo"[0] = "a"
                     // [expect-error]                     ~~~~~~~~~
                     // [expect-error]   at line 1 at the toplevel of module "main"

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in __get_prop, __set_prop, __get_subs and __set_subs.
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"
