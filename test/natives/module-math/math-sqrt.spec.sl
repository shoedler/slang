import Math

print Math // [expect] <Instance of Module>

// --- Int inputs ---
print Math.sqrt(0)    // [expect] 0
print Math.sqrt(1)    // [expect] 1
print Math.sqrt(4)    // [expect] 2
print Math.sqrt(2)    // [expect] 1.4142135623731

// ...get promoted
print Math.sqrt(0) is Float // [expect] true
print Math.sqrt(4) is Float // [expect] true
print Math.sqrt(2) is Float // [expect] true

// --- Float inputs ---
print Math.sqrt(0.0)  // [expect] 0
print Math.sqrt(1.0)  // [expect] 1
print Math.sqrt(4.0)  // [expect] 2
print Math.sqrt(2.0)  // [expect] 1.4142135623731

print Math.sqrt(0.0) is Float // [expect] true
print Math.sqrt(2.0) is Float // [expect] true

// --- Negative edge cases (NaN / domain) ---
print Math.sqrt(-1)               // [expect] -nan
print Math.sqrt(-1.0)             // [expect] -nan
