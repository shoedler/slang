print true == true     // [Expect] true
print true == false    // [Expect] false
print false == true    // [Expect] false
print false == false   // [Expect] true

// Not equal to other types.
print true == 1        // [Expect] false
print false == 0       // [Expect] false
print true == "true"   // [Expect] false
print false == "false" // [Expect] false
print false == ""      // [Expect] false

print true != true     // [Expect] false
print true != false    // [Expect] true
print false != true    // [Expect] true
print false != false   // [Expect] false

// Not equal to other types.
print true != 1        // [Expect] true
print false != 0       // [Expect] true
print true != "true"   // [Expect] true
print false != "false" // [Expect] true
print false != ""      // [Expect] true
