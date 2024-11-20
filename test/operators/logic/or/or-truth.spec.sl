// False and nil are false.
print false or "ok" // [Expect] ok
print nil or "ok" // [Expect] ok

// Everything else is true.
print true or "ok" // [Expect] true
print 0 or "ok" // [Expect] 0
print "s" or "ok" // [Expect] s
