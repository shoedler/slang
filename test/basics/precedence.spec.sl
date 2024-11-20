// * has higher precedence than +.
print 2 + 3 * 4 // [Expect] 14

// * has higher precedence than -.
print 20 - 3 * 4 // [Expect] 8

// / has higher precedence than +.
print 2 + 6 / 3 // [Expect] 4

// / has higher precedence than -.
print 2 - 6 / 3 // [Expect] 0

// < has higher precedence than ==.
print false == 2 < 1 // [Expect] true

// > has higher precedence than ==.
print false == 1 > 2 // [Expect] true

// <= has higher precedence than ==.
print false == 2 <= 1 // [Expect] true

// >= has higher precedence than ==.
print false == 1 >= 2 // [Expect] true

// or has higher precedence than ?:.
print true ? true or false : false // [Expect] true

// = has lower precedence than ?:.
let a
print a = 1 ? 2 : 3 // [Expect] 2

// 1 - 1 is not space-sensitive.
print 1 - 1 // [Expect] 0
print 1 -1  // [Expect] 0
print 1- 1  // [Expect] 0
print 1-1   // [Expect] 0

// Using () for grouping.
print (2 * (6 - (2 + 2))) // [Expect] 4
