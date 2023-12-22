let a = "a"
print a // [Expect] a

let f = !a = "value" 
print a // [Expect] value
print f // [Expect] false