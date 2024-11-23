let a = [1,2,3,4,5]

// End is exclusive
print a[0..2] // [expect] [1, 2]
// Start is inclusive
print a[1..2] // [expect] [2]

// End is optional
print a[1..]    // [expect] [2, 3, 4, 5]
print a[1..nil] // [expect] [2, 3, 4, 5]
// ... start is optional too
print a[..2] // [expect] [1, 2]
// ... so, both are optional. This literally copies the array.
print a[..] // [expect] [1, 2, 3, 4, 5]

// Negative indices are supported
print a[..-2] // [expect] [1, 2, 3]
// ... also for start
print a[-2..] // [expect] [4, 5]
// ... and for both
print a[-2..-1] // [expect] [4]

// Empty seq
print [][..] // [expect] []
print [][1..] // [expect] []
print [][..1] // [expect] []
print [][1..1] // [expect] []
print [][-1..-1] // [expect] []

// Out of bounds will clamp
print [1][0..2] // [expect] [1]
print [1][2..1] // [expect] []
print [1][1..] // [expect] []
print [1][..-1] // [expect] []
print [1][-1..] // [expect] [1]
print [1][-2..-1] // [expect] []
print [1][-1..-2] // [expect] []

// Slice of slice
print a[1..3][1..] // [expect] [3]
print a[1..3][..1] // [expect] [2]
print a[1..3][1..2] // [expect] [3]
