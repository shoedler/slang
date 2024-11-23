let a = {
    "a": 1,
    "b": 2,
    "c": 3
}

print a["d"] = 4 // [expect] 4
print a["d"] // [expect] 4

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in __get_prop, __set_prop, __get_subs and __set_subs.
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"
