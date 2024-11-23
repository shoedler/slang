// * has higher precedence than +.
print 2 + 3 * 4 // [expect] 14

// * has higher precedence than -.
print 20 - 3 * 4 // [expect] 8

// / has higher precedence than +.
print 2 + 6 / 3 // [expect] 4

// / has higher precedence than -.
print 2 - 6 / 3 // [expect] 0

// < has higher precedence than ==.
print false == 2 < 1 // [expect] true

// > has higher precedence than ==.
print false == 1 > 2 // [expect] true

// <= has higher precedence than ==.
print false == 2 <= 1 // [expect] true

// >= has higher precedence than ==.
print false == 1 >= 2 // [expect] true

// or has higher precedence than ?:.
print true ? true or false : false // [expect] true

// = has lower precedence than ?:.
let a
print a = 1 ? 2 : 3 // [expect] 2

// 1 - 1 is not space-sensitive.
print 1 - 1 // [expect] 0
print 1 -1  // [expect] 0
print 1- 1  // [expect] 0
print 1-1   // [expect] 0

// Using () for grouping.
print (2 * (6 - (2 + 2))) // [expect] 4
