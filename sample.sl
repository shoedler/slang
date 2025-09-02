print "basic data types"
print "-----------------------------------------------"

// Bool
let is_true = true
let is_false = false

print is_true     // true
print is_false    // false
print !is_true    // false (negation)
print !!is_true   // true (double negation)

// Numbers (Int and Float)
let integer = 42
let negative = -17
let float_num = 3.14159
let negative_float = -2.718

print integer        // 42
print negative       // -17
print float_num      // 3.14159
print negative_float // -2.718

print typeof(integer)      // <Int>
print typeof(float_num)    // <Float>

// Nil
let nothing = nil
print nothing       // nil
print typeof(nil)   // <Nil>

print "\nstrings"
print "-----------------------------------------------"

let simple_string = "Hello, World!"
let empty_string = ""
let unicode_string = "A~¶Þॐஃ"

print simple_string   // Hello, World!
print empty_string    // (empty)
print unicode_string  // A~¶Þॐஃ

// String escapes
let escaped = "Line 1\nLine 2"
let quoted = "\"Hello\""
print escaped     // Line 1
                  // Line 2
print quoted      // "Hello"

// They are immutable, so we can't change them. But, you can add them to get a new string
print simple_string + " How are you?"  // Hello, World! How are you?

// String char access
print simple_string[0]     // H (first element)
print simple_string[4]     // o (last element)

// ... you can also access elements using negative indices
print simple_string[-1]     // !
print simple_string[-2]     // d
print simple_string[-3]     // l

// String analysis
// ...the 'in' keyword uses the "has" method
print "H" in simple_string     // true
print "x" in simple_string     // false
// ...slicing uses the "slice" method
print simple_string[1..3]    // el
print simple_string[..2]     // He
print simple_string[2..]     // llo, World!
print simple_string[..-3]    // Hello, Wor

print "\nstring methods"
print "-----------------------------------------------"

print "1 2 34 56".split(" ")    // [1, 2, 34, 56]     --> splits a Str into a Seq by Delimiter
print "   23 ".trim()           // 23                 --> removes whitespace around the Str (' ', \f, \n, \r, \t, \v)
print "12a34".ints()            // [12, 34]           --> extracts integers from a Str
print "1.2".ints()              // [1, 2]             --> extracts integers from a Str
print "32-23".ints()            // [32, -23]          --> extracts integers from a Str
print "abc".ascii()             // [97, 98, 99]       --> gets the ASCII values of each char
print "abc".ascii_at(1)         // 98                 --> gets the ASCII value of the char at index 1
print "abc123".chars()          // [a, b, c, 1, 2, 3] --> gets the characters of the string
print "1".reps(3)               // 111                --> repeats the string 3 times

print "\nstatic string methods"
print "-----------------------------------------------"

print Str.from_ascii(98)       // b                   --> creates a Str from an ASCII value

print "\nvariables and assignment"
print "-----------------------------------------------"

// Variable declaration and assignment
let variable_a = "initial"
print variable_a  // initial

// Assignment expression returns the assigned value
let variable_b = variable_a = "new value"
print variable_a  // new value
print variable_b  // new value

// Local variables in blocks
{
  let local_var = "I'm local"
  print local_var  // I'm local
}

// Compound assignment
let variable_c = 5
variable_c += 2 // 7
variable_c -= 1 // 6
variable_c *= 2 // 12
variable_c /= 4 // 3
variable_c %= 3 // 0
print variable_c  // 0

// Inc / Dec
variable_c++ // 1
variable_c-- // 0
variable_c++ // 1
print variable_c

print "\nequality"
print "-----------------------------------------------"
print "TODO:"

print "\nconstants"
print "-----------------------------------------------"

const PI = 3.14159
const MESSAGE = "This cannot be changed"
const CONST_SEQ = [1, 2, 3]

print PI       // 3.14159
print MESSAGE  // This cannot be changed

CONST_SEQ[0] = -1 // This is allowed, only the reference is constant
print CONST_SEQ  // [-1, 2, 3]

print "\narithmetic operators"
print "-----------------------------------------------"

print 5 + 3    // 8
print 10 - 4   // 6
print 6 * 7    // 42
print 15 / 3   // 5
print 17 % 5   // 2
print -(3)     // -3

print "\ncomparison operators"
print "-----------------------------------------------"

print 5 > 3      // true
print 5 < 3      // false
print 5 >= 5     // true
print 5 <= 4     // false
print 5 == 5     // true
print 5 != 3     // true

print "\nlogical operators"
print "-----------------------------------------------"

print true and false // false
print true or false  // true
print 1 and 2 and 3  //  3
print !true          // false

print "\noperator precedence"
print "-----------------------------------------------"

// typedef enum {
//   PREC_NONE,
//   PREC_ASSIGN,      // =
//   PREC_TERNARY,     // ?:
//   PREC_OR,          // or
//   PREC_AND,         // and
//   PREC_EQUALITY,    // == !=
//   PREC_COMPARISON,  // < > <= >= is in
//   PREC_TERM,        // + -
//   PREC_FACTOR,      // * / %
//   PREC_UNARY,       // ! -
//   PREC_CALL,        // . () [] grouping with parens
//   PREC_PRIMARY
// } Precedence;

// Operator precedence as you'd expect
print 2 + 3 * 4    // 14 (multiplication first)
print (2 + 3) * 4  // 20 (parentheses first)

print "\nsequences (arrays)"
print "-----------------------------------------------"

// Sequence creation
let empty_seq = []
let numbers = [1, 2, 3, 4, 5]
let mixed_types = [1, false, nil, "hello", [1, 2, 3]]

print typeof([])     // <Seq>
print empty_seq      // []
print numbers        // [1, 2, 3, 4, 5]
print numbers.len    // 5
print empty_seq.len  // 0
print mixed_types    // [1, false, nil, hello, [1, 2, 3]]

// Sequence element access
print numbers[0]     // 1 (first element)
print numbers[4]     // 5 (last element)

// ... you can also access elements using negative indices
print numbers[-1]     // 5 (last element)
print numbers[-2]     // 4 (second to last element)

// ... and, since they are mutable, change the element
numbers[1] = 10
print numbers        // [1, 10, 3, 4, 5]

// Sequence analysis
// ...the 'in' keyword uses the "has" method
print 1 in numbers     // true
print 6 in numbers     // false
// ...slicing uses the "slice" method
print numbers[1..3]    // [10, 3]
print numbers[..2]     // [1, 10]
print numbers[2..]     // [3, 4, 5]
print numbers[..-3]    // [1, 10]

print "\nsequence methods that modify the original array"
print "-----------------------------------------------"

numbers.push(6)
print numbers        // [1, 2, 3, 4, 5, 6]

const last = numbers.pop()
print last          // 6
print numbers       // [1, 2, 3, 4, 5]

const remove_at_index = numbers.yank(1)
print remove_at_index  // 2
print numbers          // [1, 3, 4, 5]

const remove_matching_predicate = numbers.cull(fn(x) -> x % 2 == 0)
print remove_matching_predicate  // [4]
print numbers                    // [1, 3, 5]

print "\npure sequence methods"
print "-----------------------------------------------"

// Pure, because they don't modify the original array but rather return a new sequence.
const doubled = numbers.map(fn(x) -> x * 2)
print doubled       // [2, 6, 10]
print doubled == numbers // false, since it's a new sequence

print [4,5,6].pos(5)                         // 1            --> position of the first element that equals 5
print [4,5,6].pos(fn(x) -> x+3==8)           // 1            --> position of the first element that satisfies the predicate
print [4,5,6].first(fn(x) -> x > 4)          // 5            --> first element that satisfies the predicate
print [4,5,6].last(fn(x) -> x < 6)           // 5            --> last element that satisfies the predicate
print [4,5,6].each(fn(x) -> log(x))          // 4, 5, 6      --> do something with each element
print [4,5,6].map(fn(x) -> x * 2)            // [8, 10, 12]  --> map each element
print [4,5,6].sift(fn(x) -> x % 2 == 0)      // [4, 6]       --> filter even elements
print [4,5,6].join(", ")                     // "4, 5, 6"    --> join elements into a string
print [4,5,6].flip()                         // [6, 5, 4]    --> reverse the order of elements
print [4,5,6].every(fn(x) -> x is Num)       // true         --> evaluate a predicate for all elements
print [4,5,6].some(fn(x) -> x is Str)        // false        --> check if any element satisfies the predicate
print [4,5,6].fold(0, fn(acc, x) -> acc + x) // 15           --> fold to a single value (in this case, sum)
print [4,5,6].count(4)                       // 1            --> count occurrences of 4
print [4,5,6].count(fn(x) -> x > 4)          // 2            --> count elements satisfying the predicate
print [4,5,6].concat([7])                    // [4, 5, 6, 7] --> concatenate with another seq
print [4,5,6].order(fn(a, b) -> b - a)       // [6, 5, 4]    --> sort elements using a comparator
print [6,4,5].sort()                         // [4, 5, 6]    --> sort elements in ascending order - uses the "lt" method
print [4,5,6].min()                          // 4            --> find the minimum element - uses the "lt" method
print [4,5,6].sum()                          // 15           --> find the sum of elements - uses the "add" method

print "\ntuples"
print "-----------------------------------------------"

// Tuple creation (immutable sequences)
const empty_tuple = (,)          // Empty tuple
const single_tuple = (1,)        // Single element tuple
const pair = (1, 2)             // Regular tuple
const mixed_tuple = (1, false, nil, "hello", [1, 2, 3])

print typeof(empty_tuple)   // <Tuple>
print empty_tuple.len       // 0
print single_tuple.len      // 1
print pair                  // (1, 2)
print mixed_tuple           // (1, false, nil, hello, [1, 2, 3])

// Tuple access
print pair[0]               // 1
print pair[1]               // 2

// Tuple analysis
// ...the 'in' keyword uses the "has" method
print 1 in pair        // true
print 0 in pair        // false
// ...slicing uses the "slice" method
print pair[1..]        // (2)
print pair[0..1]       // (1)
print pair[..-1]       // (1)

// They are equal, if the elements are equal
print pair == (1, 2)        // true
print pair == (2, 1)        // false

// ...they support all of the "pure" sequence methods

print "\nobjects"
print "-----------------------------------------------"

// Object creation
let empty_obj = {}
let person = {
  "name": "Alice",
  "age": 30,
  "city": "New York"
}

print empty_obj    // {}
print person       // {age: 30, city: New York, name: Alice}

// Object property access
print person["name"]  // Alice
print person.age      // 30 (dot notation also works)

// Object property assignment
person["occupation"] = "Engineer"
person.salary = 75000
print person         // {age: 30, city: New York, name: Alice, occupation: Engineer, salary: 75000}

// Check if property exists
print "name" in person      // true
print "height" in person    // false

print "\nobject methods"
print "-----------------------------------------------"

print person.keys()   // ["name", "age", "city", "occupation", "salary"] --> get all keys - order not guaranteed
print person.values() // ["Alice", 30, "New York", "Engineer", 75000]    --> get all values - order not guaranteed
print person.entries() // [["name", "Alice"], ["age", 30], ...]          --> get all key-value pairs - order not guaranteed

print "\nconditional statements"
print "-----------------------------------------------"

// Basic if statement
let score = 85
if score >= 90 {
  print "Grade: A"
} else if score >= 80 {
  print "Grade: B"
} else if score >= 70 {
  print "Grade: C"
} else {
  print "Grade: F"
}
// Grade: B

// Single-line if
if score > 50 print "Passing"  // Passing

// Assignment in condition
let user_input = false
if user_input = true {
  print "Input was set to true"  // Input was set to true
}

// Ternary operator
let status = score >= 60 ? "Pass" : "Fail"
print status  // Pass


print "\nloops"
print "-----------------------------------------------"

// While loop
print "While loop countdown:"
let countdown = 3
while countdown > 0 {
  print countdown  // 3, 2, 1
  countdown = countdown - 1
}

// For loop
print "For loop counting:"
for let i = 0; i < 3; i = i + 1; {
  print i        // 0, 1, 2
}

// For loop with no variable declaration
let j = 0
for ; j < 3; j = j + 1; {
  print "j is " + j  // j is 0, j is 1, j is 2
}

// ...in fact, everything is optional, here's an infinite loop with break
print "Break example:"
let counter = 0
for ;;; {
  if counter >= 2 break
  print counter    // 0, 1
  counter = counter + 1
}

// Skip example
print "Skip example:"
for let k = 0; k < 5; k = k + 1; {
  if k % 2 == 0 skip
  print k          // 1, 3
}

print "\nfunctions"
print "-----------------------------------------------"

// Named function
fn greet(name) {
  print "Hello, " + name + "!"
}

greet("Alice")  // Hello, Alice!

// Function with return value
fn add(a, b) {
  ret a + b
}

let result = add(5, 3)
print result    // 8

// Anonymous function
let multiply = fn(x, y) -> x * y
print multiply(4, 5)  // 20

// Functions are first-class citizens and can be assigned to variables
let square = fn(x) -> x * x
print square(7)  // 49

// Higher-order functions
fn apply_twice(func, value) {
  ret func(func(value))
}

print apply_twice(square, 3)  // 81 (3^2 = 9, 9^2 = 81)

// Recursion
fn factorial(n) {
  if n <= 1 ret 1
  ret n * factorial(n - 1)
}

print factorial(5)  // 120

// Closures
fn make_counter() {
  let count = 0
  ret fn() {
    count = count + 1
    ret count
  }
}

let counter1 = make_counter()
let counter2 = make_counter()
print counter1()  // 1
print counter1()  // 2
print counter2()  // 1 (separate closure)

print "\ndestructuring"
print "-----------------------------------------------"

// Sequence destructuring
let [first, second, third] = [10, 20, 30, 40]
print first   // 10
print second  // 20
print third   // 30

// Rest parameters
let [head, ...tail] = [1, 2, 3, 4, 5]
print head  // 1
print tail  // [2, 3, 4, 5]

// Object destructuring
let {name, age} = {"name": "Bob", "age": 25, "city": "Boston"}
print name  // Bob
print age   // 25

// Tuple destructuring
let (x, y, z) = (100, 200, 300)
print x  // 100
print y  // 200
print z  // 300

// String destructuring
let [char1, char2] = "Hi"
print char1  // H
print char2  // i

print "\nclasses and object-oriented programming"
print "-----------------------------------------------"

// Basic class definition
cls Animal {
  ctor(name, species) {
    this.name = name
    this.species = species
  }
  
  fn speak() {
    print this.name + " makes a sound"
  }
  
  fn info() {
    ret this.name + " is a " + this.species
  }
}

// Create instance
let dog = Animal("Buddy", "Dog")
dog.speak()         // Buddy makes a sound
print dog.info()    // Buddy is a Dog
print dog.name      // Buddy

// Class inheritance
cls Dog : Animal {
  ctor(name, breed) {
    base.ctor(name, "Dog")
    this.breed = breed
  }
  
  fn speak() {
    print this.name + " barks: Woof!"
  }
  
  fn get_breed() {
    ret this.breed
  }
}

let retriever = Dog("Max", "Golden Retriever")
retriever.speak()            // Max barks: Woof!
print retriever.info()       // Max is a Dog
print retriever.get_breed()  // Golden Retriever

// Method binding
let speak_method = retriever.speak
speak_method()               // Max barks: Woof!

print "\ntype checking and introspection"
print "-----------------------------------------------"

// typeof function
print typeof(42)           // <Int>
print typeof(3.14)         // <Float>
print typeof("hello")      // <Str>
print typeof([1, 2, 3])    // <Seq>
print typeof((1, 2))       // <Tuple>
print typeof({})           // <Obj>
print typeof(fn -> nil)    // <Fn>
print typeof(Animal)       // <Class>
print typeof(dog)          // <Instance of Animal>

// Type checking with 'is'
print 42 is Int           // true
print "hello" is Str      // true
print dog is Animal       // true
print retriever is Dog    // true
print retriever is Animal // true (inheritance)

// ...or, 'is not'
print retriever is not Animal // false

print "\noperator overloading"
print "-----------------------------------------------"

// Custom class with operator overloading
cls Vector {
  ctor(x, y) {
    this.x = x
    this.y = y
  }
  
  fn add(other) {
    ret Vector(this.x + other.x, this.y + other.y)
  }
  
  fn sub(other) {
    ret Vector(this.x - other.x, this.y - other.y)
  }
  
  fn mul(scalar) {
    ret Vector(this.x * scalar, this.y * scalar)
  }
  
  fn to_str() {
    ret "Vector(" + this.x + ", " + this.y + ")"
  }
  
  fn get_subs(index) {
    if index == 0 ret this.x
    if index == 1 ret this.y
    ret nil
  }
}

let v1 = Vector(1, 2)
let v2 = Vector(3, 4)
let v3 = v1 + v2        // Uses __add
let v4 = v2 - v1        // Uses __sub
let v5 = v1 * 3         // Uses __mul

print v1                // Vector(1, 2) - uses __to_str
print v3                // Vector(4, 6)
print v4                // Vector(2, 2)
print v5                // Vector(3, 6)
print v1[0]             // 1 - uses __get_subs
print v1[1]             // 2 - uses __get_subs

// ...you can also override 'has' (for the 'in' operator), 'slice', for the '[]' operator 
// and all the comparators like 'lt', 'lteq' etc.

print "\nerror handling"
print "-----------------------------------------------"

// Basic try-catch
try {
  let dangerous = nil / 2   // This will cause an error
  dangerous + 2 // (only to silence the not-used warning)
} catch {
  print "Caught error: " + error  // Caught error: Type Nil does not support "div".
}

print "Program continues"  // Program continues

// Throwing a custom error involves overriding a classes "to_str" method
cls CustomError {
  ctor(message) {
    this.message = message
  }
  
  fn to_str() {
    ret "CustomError: " + this.message
  }
}

try {
  throw CustomError("Something went wrong!")
} catch {
  print "Caught: " + error  // Caught: CustomError: Something went wrong!
}

// Try without catch (error is silenced)
try {
  nil + "hello"  // This would normally cause an error
}
print "Error was silenced"  // Error was silenced

// There's also Try-expressions. These are pretty useful!
print try 1 / 0 else error // Some error about division by zero

const fallback_result = try 1 / 0 else 42 // 42
print fallback_result       // 42

print "\nmodule system"
print "-----------------------------------------------"

// Note: These examples assume module files exist
// import module_name                     // Import entire module
// import { func1, Class1 } from module   // Import specific items
// let module_alias = import module       // Import with alias
// import { foo } from "../../module.sl"  // Import from a relative path (absolute works too)

// Module exports are implicit - all top-level declarations are exported

print "\nnative functions"
print "-----------------------------------------------"

// Built-in functions
// ...there's 'clock', a which returns the current execution time in seconds (Float)
print typeof(clock)      // <Fn> - clock is a native function
let start_time = clock()
let end_time = clock()
print "Time difference: " + (end_time - start_time)  // Very small number

// ...'log', which is an alias for 'print' - but, can be used as an expression (whereas 'print' is a statement)
print typeof(log)       // <Fn> - log is a native function
log("Hello, World!")    // Hello, World!
log(1,2,3)              // 1 2 3 --> accepts arbitrary number of arguments
print log(1)            // nil --> log returns nil

// ...'typeof', which returns the type of a value
print typeof(typeof)    // <Fn> - typeof is a native function
print typeof(PI)        // Float
print typeof("Hello")   // String
print typeof([1, 2, 3]) // Array

// ...'cwd', which returns the current working directory (String)
print typeof(cwd)       // <Fn> - cwd is a native function
print cwd()             // /home/user/project (or something the like)

print "\nbuilt-in modules"
print "-----------------------------------------------"
print "TODO:"