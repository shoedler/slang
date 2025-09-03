print "\nvariables and assignment"
print "-----------------------------------------------"

// Variable declaration and assignment
let variable_a = "initial"
print variable_a  // initial

// Assignment expressions return the assigned value
let variable_b = variable_a = "new value"
//               ^----------------------^--> assignment expression

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

print "\nconstants"
print "-----------------------------------------------"

const PI = 3.14159
const MESSAGE = "This cannot be changed"
const CONST_SEQ = [1, 2, 3]

print PI       // 3.14159
print MESSAGE  // This cannot be changed

CONST_SEQ[0] = -1 // This is allowed, only the reference is constant
print CONST_SEQ  // [-1, 2, 3]

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

print typeof("foo") // <Str>

// Str creation (immutable strings of characters)
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

// Strs are immutable, so we can't change them. But, you can add them to get a new string
print simple_string + " How are you?"  // Hello, World! How are you?

print "\nstring analysis"
print "-----------------------------------------------"

// ...you can access its "len" prop to get the amount of chars
print simple_string.len    // 13
// ...you can access individual chars with indexing
print simple_string[0]     // H (first element)
print simple_string[4]     // o (last element)
// ...you can also access elements using negative indices
print simple_string[-1]     // !
print simple_string[-2]     // d
print simple_string[-3]     // l
// ...the 'in' keyword uses the "has" method
print "H" in simple_string  // true
print "x" in simple_string  // false
// ...slicing uses the "slice" method
print simple_string[1..3]   // el
print simple_string[..2]    // He
print simple_string[2..]    // llo, World!
print simple_string[..-3]   // Hello, Wor

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


print "\nequality"
print "-----------------------------------------------"
print "TODO:"

print "\ntruthiness"
print "-----------------------------------------------"

// ...false & nil are falsy
print false    and true // false
print nil      and true // false
// ...everything else is truthy
print 0        and true // true
print ""       and true // true
print 0.0      and true // true
print []       and true // true
print (,)      and true // true
print {1:2}    and true // true
print fn -> 1  and true // true

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

print typeof([]) // <Seq>

// Sequence creation
let empty_seq = []
let numbers = [1, 2, 3, 4, 5]
let mixed_types = [1, false, nil, "hello", [1, 2, 3]]
print empty_seq      // []
print numbers        // [1, 2, 3, 4, 5]
print mixed_types    // [1, false, nil, hello, [1, 2, 3]]

print "\nsequences analysis"
print "-----------------------------------------------"

// ...you can access its "len" prop to get the amount of items
print numbers.len   // 5
// ...you can access individual items with indexing
print numbers[0]    // 1 (first element)
print numbers[4]    // 5 (last element)
// ... you can also access elements using negative indices
print numbers[-1]   // 5 (last element)
print numbers[-2]   // 4 (second to last element)
// ... and, since they are mutable, change the element
numbers[1] = 10
print numbers       // [1, 10, 3, 4, 5]
// ...the 'in' keyword uses the "has" method
print 1 in numbers  // true
print 6 in numbers  // false
// ...slicing uses the "slice" method
print numbers[1..3] // [10, 3]
print numbers[..2]  // [1, 10]
print numbers[2..]  // [3, 4, 5]
print numbers[..-3] // [1, 10]

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

// "Pure", because they don't modify the original array but rather return a new sequence.

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

print typeof((1,2,3))   // <Tuple>

// Tuple creation (immutable sequences)
const empty_tuple = (,)   // Empty tuple
const single_tuple = (1,) // Single element tuple
const pair = (1, 2)       // Regular tuple
const mixed_tuple = (1, false, nil, "hello", [1, 2, 3])
print empty_tuple
print single_tuple
print pair                  // (1, 2)
print mixed_tuple           // (1, false, nil, hello, [1, 2, 3])

// The main thing about them is that they are not compared/hashed using their memory-pointer, but rather
// their contents. They are equal, if the elements are equal
print pair == (1, 2)        // true
print pair == (2, 1)        // false

print "\ntuple analysis"
print "-----------------------------------------------"

// ...you can access its "len" prop to get the amount of items contained within it
print pair.len   // 2
// ...you can access elements using indexing
print pair[0]    // 1
print pair[1]    // 2
// ...you can also use negative indices
print pair[-1]   // 2
print pair[-2]   // 1
// ...the 'in' keyword uses the "has" method
print 1 in pair  // true
print 0 in pair  // false
// ...slicing uses the "slice" method
print pair[1..]  // (2)
print pair[0..1] // (1)
print pair[..-1] // (1)


// ...they support all of the "pure" sequence methods

print "\nobjects"
print "-----------------------------------------------"

print typeof({})  // <Obj>

// Object creation
let empty_obj = {}
let person = {
  "name": "Alice",
  "age": 30,
  "city": "New York"
}
print empty_obj    // {}
print person       // {age: 30, city: New York, name: Alice}

// What's special about them, is that anything can be used as a key. From primitives
// all the way to complex objects / reference types:
const fn_key = fn -> 1
const seq_key = [4,5,6]
const tuple_key = (4,5,6)
const obj_key = {3:4}

const crazy_obj = {
    1: 2, 
    true: false,
    nil: "null", 
    "hello": "world",
    fn -> 1: fn -> 2, // Unreachable, because we have no reference to this function
    fn_key: fn -> 2, // Reachable

    [1,2,3]: [4,5,6], // Unreachable, because we have no reference to this array
    seq_key: [4,5,6], // Reachable

    (1,2,3): (3,2,1), // Also reachable, because tuples are hashable
    tuple_key: (6,5,4), // Reachable. (This actually overwrites the matching tuple-entries)

    {1:2}: {3:4}, // Unreachable, because we have no reference to this object
    obj_key: {3:4} // Reachable
}
print crazy_obj[1]         // 2
print crazy_obj[true]      // false
print crazy_obj[nil]       // "null"
print crazy_obj["hello"]   // "world"
print crazy_obj[fn_key]    // <Fn $anon_fn$>
print crazy_obj[seq_key]   // [4,5,6]
print crazy_obj[tuple_key] // (6,5,4)
print crazy_obj[(1,2,3)]   // (3,2,1)
print crazy_obj[obj_key]   // {3:4}
// ...but, it's only possible with indexing notation, not dot notation - except for strings.
print crazy_obj.hello                 // "world"
print try crazy_obj.fn_key else error // Property 'fn_key' does not exist on value of type Obj.

print "\ntuple analysis"
print "-----------------------------------------------"

// ...you can access its "len" prop to get the amount of kvps 
print person.len      // 3
// ...you can access object properties using both indexing and dot notation
print person["name"]  // Alice
print person.age      // 30 (dot notation also works)
// ...and, since they are mutable, you can add new, and override properties
person["occupation"] = "Engineer"
person.salary = 75000 // Didn't exist yet
print person          // {age: 30, city: New York, name: Alice, occupation: Engineer, salary: 75000}
// ...you can check if a prop exists by key using the "in" keyword (uses the "has" method)
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

const result = add(5, 3)
print result    // 8

// Anonymous function
const multiply = fn(x, y) -> x * y
print multiply(4, 5)  // 20

// ...params are optional, which means parens are optional allowing for very concise code:
const answer = fn -> 42
print answer() // 42

// Functions are first-class citizens and can be assigned to variables
const square = fn(x) -> x * x
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
// ...instances are Objs
print dog is Animal // true
print dog is Obj    // true

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
print retriever is Obj    // true (base-type)

// ...or, 'is not'
print retriever is not Animal // false
print retriever is not Obj    // false

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

// ...you can also override 'has' (for the 'in' operator), 'slice', for the '[]' operator 
// and all the comparators like 'lt', 'lteq' etc.
// dot- and indexing-access isn't currently overwritable.

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

// ...there's 'clock', a which returns the current execution time in seconds (Float)
print typeof(clock)      // <Fn> - clock is a native function
let start_time = clock()
let end_time = clock()
print "Time difference: " + (end_time - start_time)  // Very small number

// ...'log', which is an alias for 'print' - but, it is an expression (whereas 'print' is a statement)
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

// ...there's the "Debug" module
import Debug
print Debug.stack() // Get the current stack of the Vm as a Seq- this is a copy and changes do absolutely nothing.
print Debug.version() // Get the current version of the Vm
print Debug.modules() // Get an Obj of all loaded modules.
//Debug.heap()  // Prints the current heap linked-list to the console

// ...there's the "Gc" module
import Gc
print Gc.collect() // Force a garbage collection cycle- returns the number of freed bytes from that cycle
print Gc.stats()   // Get the most recent Gc-statistics and markers
//Gc.stress(Bool) // Toggle Stress-mode (default is off). "Stress" means to force a cycle on every allocation.

// ...there's the "Perf" module
import Perf
print Perf.now() // Get the current high-resolution time in Seconds (Float)
print Perf.since(Perf.now()) // Get the time since the given timestamp

// ...there's the "File" module
import File
const sample_path = File.join_path(cwd(), "sample.sl") // Joins two paths using system delimiter
print File.read(sample_path).len                       // Read a file at a given path (absolute)
print File.exists(sample_path)                         // Checks whether a file exists at a given absolute path
//print File.write("some/path", "some content")        // Writes content to a file at a given path (absolute)

print "\nclassic stuff & samples written in slang"
print "-----------------------------------------------"

// Classic Fibonacci sequence
fn fib(n) -> n <= 1 ? n : fib(n-1) + fib(n-2)

let start = Perf.now()
for let i = 0; i < 5; ++i; print fib(30)
print "elapsed: " + Perf.since(start) + "s"

// "Classic" FizzBuzz
for let i = 1; i < 101; ++i; print [i,"Fizz","Buzz","FizzBuzz"][(i%3<1?1:0)+(i%5<1?2:0)]
