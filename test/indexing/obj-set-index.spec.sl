let a = {
    "a": 1,
    "b": 2,
    "c": 3
}

print a["c"] = 9 // [Expect] 9
print a["c"] // [Expect] 9

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in value_get_property, value_set_property, value_get_index and value_set_index
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"