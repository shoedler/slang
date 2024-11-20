import a

// Was declared as const in a.sl
print a.A // [Expect] 42
// Was declared as let in a.sl
print a.B // [Expect] 69

a.A = nil // Still allowed, assigning to "a" would not be allowed
a.B = nil

print a.A // [Expect] nil
print a.B // [Expect] nil
