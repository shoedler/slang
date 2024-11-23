import a

// Was declared as const in a.sl
print a.A // [expect] 42
// Was declared as let in a.sl
print a.B // [expect] 69

a.A = nil // Still allowed, assigning to "a" would not be allowed
a.B = nil

print a.A // [expect] nil
print a.B // [expect] nil
