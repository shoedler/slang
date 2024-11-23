// No arguments
print try {}.has() else error // [expect] Expected 1 argument but got 0.

// Empty string
print "".has("hello") // [expect] false
print "".has("")      // [expect] true

// Full match
print "hello".has("hello") // [expect] true

// Match start
print "hello".has("he") // [expect] true

// Match end
print "hello".has("lo") // [expect] true

// No match
print "hello".has("lol") // [expect] false

// Partial match
print "hello".has("ell") // [expect] true

// Does not work with callables or any non-string types
print try ("lol".has(fn (x) -> x is Str)) else error // [expect] Expected argument 0 of type Str but got Fn.
