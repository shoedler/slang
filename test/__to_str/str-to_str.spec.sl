fn quotify (str) -> "\"" + str + "\""

print quotify("").to_str()     // [Expect] ""
print quotify("a").to_str()    // [Expect] "a"
print quotify("abc").to_str()  // [Expect] "abc"
print quotify("a\"b").to_str() // [Expect] "a"b"
print quotify("a\\b").to_str() // [Expect] "a\b"

print quotify("")     // [Expect] ""
print quotify("a")    // [Expect] "a"
print quotify("abc")  // [Expect] "abc"
print quotify("a\"b") // [Expect] "a"b"
print quotify("a\\b") // [Expect] "a\b"

print typeof("").to_str().to_str() // [Expect] <Class Str>