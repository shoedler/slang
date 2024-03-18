// Bound methods have identity equality.
cls Foo {}
cls Bar {}

print Foo == Foo // [Expect] true
print Foo == Bar // [Expect] false
print Bar == Foo // [Expect] false
print Bar == Bar // [Expect] true

print Foo == "Foo" // [Expect] false
print Foo == nil   // [Expect] false
print Foo == 123   // [Expect] false
print Foo == true  // [Expect] false
  
// Different instances of the same class have identity equality. 
print Foo() == Foo() // [Expect] false

let a = Foo()
let b = a
print a == a // [Expect] true
print a == b // [Expect] true

a = []
print a == b // [Expect] false
print b      // [Expect] <Instance of Foo>