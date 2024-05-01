print "foo"[0] = "a" // [ExpectError] Uncaught error: Type Str does not support set-indexing with Int.
                     // [ExpectError]      1 | print "foo"[0] = "a"
                     // [ExpectError]                     ~~~~~~
                     // [ExpectError]   at line 1 at the toplevel of module "main"

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in value_get_property, value_set_property, value_get_index and value_set_index
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"