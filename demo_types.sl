// Type System Demo - showcases the implemented typechecker features

// ✅ Basic type annotations
let age: int = 25
let height: flt = 5.9  
let name: str = "Alice"
let is_student: bool = true

// ✅ Type inference (no annotations needed)
let count = 100        // inferred as int
let pi = 3.14159      // inferred as flt  
let greeting = "Hi"   // inferred as str
let ready = false     // inferred as bool

// ✅ Arithmetic operations with type checking
let sum: int = 10 + 20              // int + int → int
let avg: flt = 15.5 + 2             // flt + int → flt  
let total: flt = age + height       // int + flt → flt (type coercion)
let full_name: str = "Mr. " + name  // str + str → str

// ✅ Comparison operations  
let is_adult: bool = age >= 18      // int >= int → bool
let is_tall: bool = height > 6.0    // flt > flt → bool  
let name_check: bool = name == "Alice"  // str == str → bool

// ✅ Complex expressions
let result: flt = (age + 5) * 1.5   // (int + int) * flt → flt
let discount: bool = (age < 65) && is_student  // bool && bool → bool

// ✅ Type coercion examples  
let precise: flt = 42               // int → flt (allowed)

// ❌ These would cause type errors (commented out):
// let invalid1: int = 3.14         // flt → int (not allowed)
// let invalid2: str = 42           // int → str (not allowed)  
// let invalid3: int = "hello" + "world"  // str → int (not allowed)
// let invalid4: bool = age + name  // int + str (operation not supported)

print "Type system demo completed!"
print "Age:", age
print "Height:", height  
print "Name:", name
print "Sum:", sum
print "Average:", avg
print "Is adult:", is_adult