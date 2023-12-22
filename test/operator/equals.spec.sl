print nil == nil // [Expect] true

print true == true // [Expect] true
print true == false // [Expect] false

print 1 == 1 // [Expect] true
print 1 == 2 // [Expect] false

print "str" == "str" // [Expect] true
print "str" == "ing" // [Expect] false

print nil == false // [Expect] false
print false == 0 // [Expect] false
print 0 == "0" // [Expect] false
