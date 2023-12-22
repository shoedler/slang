let nan = 0/0

print nan == 0 // [Expect] false
print nan != 1 // [Expect] true

// NaN is not equal to self.
print nan == nan // [Expect] false
print nan != nan // [Expect] true
