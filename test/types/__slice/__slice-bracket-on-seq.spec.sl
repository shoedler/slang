let a = [1,2,3,4,5]

// End is exclusive
print a[0..2] // [Expect] [1, 2]
// Start is inclusive
print a[1..2] // [Expect] [2]

// End is optional
print a[1..]    // [Expect] [2, 3, 4, 5]
print a[1..nil] // [Expect] [2, 3, 4, 5]
// ... start is optional too
print a[..2] // [Expect] [1, 2]
// ... so, both are optional. This literally copies the array.
print a[..] // [Expect] [1, 2, 3, 4, 5]

// Negative indices are supported
print a[..-2] // [Expect] [1, 2, 3]
// ... also for start
print a[-2..] // [Expect] [4, 5]
// ... and for both
print a[-2..-1] // [Expect] [4]

// Empty seq
print [][..] // [Expect] []
print [][1..] // [Expect] []
print [][..1] // [Expect] []
print [][1..1] // [Expect] []
print [][-1..-1] // [Expect] []

// Out of bounds will clamp
print [1][0..2] // [Expect] [1]
print [1][2..1] // [Expect] []
print [1][1..] // [Expect] []
print [1][..-1] // [Expect] []
print [1][-1..] // [Expect] [1]
print [1][-2..-1] // [Expect] []
print [1][-1..-2] // [Expect] []

// Slice of slice
print a[1..3][1..] // [Expect] [3]
print a[1..3][..1] // [Expect] [2]
print a[1..3][1..2] // [Expect] [3]
