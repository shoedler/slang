// No arguments
print try {}.has() else error // [Expect] Expected 1 argument but got 0.

// Empty string
print "".has("hello") // [Expect] false
print "".has("")      // [Expect] true

// Full match
print "hello".has("hello") // [Expect] true

// Match start
print "hello".has("he") // [Expect] true

// Match end
print "hello".has("lo") // [Expect] true

// No match
print "hello".has("lol") // [Expect] false

// Partial match
print "hello".has("ell") // [Expect] true

// Does not work with callables or any non-string types
print try ("lol".has(fn (x) -> x is Str)) else error // [Expect] Expected argument 0 of type Str but got Fn.
