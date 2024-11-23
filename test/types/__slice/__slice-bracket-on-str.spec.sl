let a = "12345"
fn quotify(str) -> "\"" + str + "\""

// End is exclusive
print quotify(a[0..2]) // [expect] "12"
// Start is inclusive
print quotify(a[1..2]) // [expect] "2"

// End is optional
print quotify(a[1..]   ) // [expect] "2345"
print quotify(a[1..nil]) // [expect] "2345"
// ... start is optional too
print quotify(a[..2])    // [expect] "12"
// ... so, both are optional. This literally copies the array.
print quotify(a[..])     // [expect] "12345"

// Negative indices are supported
print quotify(a[..-2])   // [expect] "123"
// ... also for start
print quotify(a[-2..])   // [expect] "45"
// ... and for both
print quotify(a[-2..-1]) // [expect] "4"

// Empty str
print quotify(""[..])     // [expect] ""
print quotify(""[1..])    // [expect] ""
print quotify(""[..1])    // [expect] ""
print quotify(""[1..1])   // [expect] ""
print quotify(""[-1..-1]) // [expect] ""

// Out of bounds will clamp
print quotify("1"[0..2])   // [expect] "1"
print quotify("1"[2..1])   // [expect] ""
print quotify("1"[1..])    // [expect] ""
print quotify("1"[..-1])   // [expect] ""
print quotify("1"[-1..])   // [expect] "1"
print quotify("1"[-2..-1]) // [expect] ""
print quotify("1"[-1..-2]) // [expect] ""

// Slice of slice
print quotify(a[1..3][1..])  // [expect] "3"
print quotify(a[1..3][..1])  // [expect] "2"
print quotify(a[1..3][1..2]) // [expect] "3"
