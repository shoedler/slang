fn quotify (str) -> "\"" + str + "\""

print quotify("").to_str()     // [expect] ""
print quotify("a").to_str()    // [expect] "a"
print quotify("abc").to_str()  // [expect] "abc"
print quotify("a\"b").to_str() // [expect] "a"b"
print quotify("a\\b").to_str() // [expect] "a\b"

print quotify("")     // [expect] ""
print quotify("a")    // [expect] "a"
print quotify("abc")  // [expect] "abc"
print quotify("a\"b") // [expect] "a"b"
print quotify("a\\b") // [expect] "a\b"

print typeof("").to_str().to_str() // [expect] <Str>