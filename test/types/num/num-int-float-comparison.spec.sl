print 0x2 == 2.0  // [expect] true
print 0b10 == 2.0 // [expect] true
print 0o2 == 2.0  // [expect] true
print 2.0 == 2    // [expect] true
print 2.0 == 2.0  // [expect] true

print Float("2") == 0x2 // [expect] true
print Int("2") == 2.0   // [expect] true
