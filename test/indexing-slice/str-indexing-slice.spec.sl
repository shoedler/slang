let a = "12345"
fn quotify(str) -> "\"" + str + "\""

// End is exclusive
print quotify(a[0..2]) // [Expect] "12"
// Start is inclusive
print quotify(a[1..2]) // [Expect] "2"

// End is optional
print quotify(a[1..]   ) // [Expect] "2345"
print quotify(a[1..nil]) // [Expect] "2345"
// ... start is optional too
print quotify(a[..2])    // [Expect] "12"
// ... so, both are optional. This literally copies the array.
print quotify(a[..])     // [Expect] "12345"

// Negative indices are supported
print quotify(a[..-2])   // [Expect] "123"
// ... also for start
print quotify(a[-2..])   // [Expect] "45"
// ... and for both
print quotify(a[-2..-1]) // [Expect] "4"

// Empty str
print quotify(""[..])     // [Expect] ""
print quotify(""[1..])    // [Expect] ""
print quotify(""[..1])    // [Expect] ""
print quotify(""[1..1])   // [Expect] ""
print quotify(""[-1..-1]) // [Expect] ""

// Out of bounds will clamp
print quotify("1"[0..2])   // [Expect] "1"
print quotify("1"[2..1])   // [Expect] ""
print quotify("1"[1..])    // [Expect] ""
print quotify("1"[..-1])   // [Expect] ""
print quotify("1"[-1..])   // [Expect] "1"
print quotify("1"[-2..-1]) // [Expect] ""
print quotify("1"[-1..-2]) // [Expect] ""

// Slice of slice
print quotify(a[1..3][1..])  // [Expect] "3"
print quotify(a[1..3][..1])  // [Expect] "2"
print quotify(a[1..3][1..2]) // [Expect] "3"
