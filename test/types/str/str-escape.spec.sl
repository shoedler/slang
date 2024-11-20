print "1\n2" 
// [Expect] 1
// [Expect] 2

// Escape sequences adjacent to the string literal quoation marks.
print "\"hello\"" // [Expect] "hello"

print "\'\"" // [Expect] '"

// TODO: Add \b, \f, \t