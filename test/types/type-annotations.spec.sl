// Test basic type annotations
let a: int = 42
let b: str = "hello"  
let c: flt = 3.14
let d: bool = true

print a  // [expect] 42
print b  // [expect] hello
print c  // [expect] 3.14
print d  // [expect] true

// Test type inference
let e = 123
let f = "world"
let g = 2.718

print e  // [expect] 123
print f  // [expect] world
print g  // [expect] 2.718

// Test arithmetic type operations
let h = a + 10
let i = c + 1.0

print h  // [expect] 52
print i  // [expect] 4.14