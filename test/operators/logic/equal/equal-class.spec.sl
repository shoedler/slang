// Bound methods have identity equality.
cls Foo {}
cls Bar {}

print Foo == Foo // [expect] true
print Foo == Bar // [expect] false
print Bar == Foo // [expect] false
print Bar == Bar // [expect] true

print Foo == "Foo" // [expect] false
print Foo == nil   // [expect] false
print Foo == 123   // [expect] false
print Foo == true  // [expect] false
  
// Different instances of the same class have identity equality. 
print Foo() == Foo() // [expect] false

let a = Foo()
let b = a
print a == a // [expect] true
print a == b // [expect] true

a = []
print a == b // [expect] false
print b      // [expect] <Instance of Foo>