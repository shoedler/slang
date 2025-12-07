let time = clock()
let another_time = clock()

const t = (another_time - time)
print t is Num  // [expect] true
print t < 0.01  // [expect] true
print t >= 0    // [expect] true
print clock     // [expect] <NativeFn clock>
