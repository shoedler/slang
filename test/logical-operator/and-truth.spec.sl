// False and nil are false.
print false and "bad" // [Expect] false
print nil and "bad" // [Expect] nil

// Everything else is true.
print true and "ok" // [Expect] ok
print 0 and "ok" // [Expect] ok
print "" and "ok" // [Expect] ok
