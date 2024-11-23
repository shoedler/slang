let a = [1,2,3]
print a[0] // [expect] 1

// Negative index should be allowed
print a[-1] // [expect] 3

// Out of bound index should return nil
print a[3] // [expect] nil
print a[-4] // [expect] nil

// Stack check. Only the toplevel fn should be in the stack at this point.
// Added this bc there were some bugs in __get_prop, __set_prop, __get_subs and __set_subs.
import Debug
let stack = Debug.stack()
if stack.len != 1 throw "Stack should only contain the toplevel function"
if !(stack[0] is Fn) or !(stack[0].__name == "main") throw "Stack should only contain the toplevel function"
