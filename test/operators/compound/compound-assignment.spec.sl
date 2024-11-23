
let a = [1]

print a[0]+=1      // [expect] 2
print a[0]         // [expect] 2
print a[0]-=1      // [expect] 1
print a[0]         // [expect] 1
print a[0] *= 2.5  // [expect] 2.5
print a[0]         // [expect] 2.5
print a[0] %= 2    // [expect] 0.5
print a[0]         // [expect] 0.5
print a[0] /= 2    // [expect] 0.25
print a[0]         // [expect] 0.25

let b = 1
print b+=1         // [expect] 2
print b            // [expect] 2
print b-=1         // [expect] 1
print b            // [expect] 1
print b *= 2.5     // [expect] 2.5
print b            // [expect] 2.5
print b %= 2       // [expect] 0.5
print b            // [expect] 0.5
print b /= 2       // [expect] 0.25
print b            // [expect] 0.25

let c = {"a": 1}
print c.a+=1       // [expect] 2
print c.a          // [expect] 2
print c.a-=1       // [expect] 1
print c.a          // [expect] 1
print c.a *= 2.5   // [expect] 2.5
print c.a          // [expect] 2.5
print c.a %= 2     // [expect] 0.5
print c.a          // [expect] 0.5
print c.a /= 2     // [expect] 0.25
print c.a          // [expect] 0.25

let d = [[1]]
print d[0][0]+=1      // [expect] 2
print d[0][0]         // [expect] 2
print d[0][0]-=1      // [expect] 1
print d[0][0]         // [expect] 1
print d[0][0] *= 2.5  // [expect] 2.5
print d[0][0]         // [expect] 2.5
print d[0][0] %= 2    // [expect] 0.5
print d[0][0]         // [expect] 0.5
print d[0][0] /= 2    // [expect] 0.25
print d[0][0]         // [expect] 0.25

let e = {"a": {"b": 1}}
print e.a.b+=1      // [expect] 2
print e.a.b         // [expect] 2
print e.a.b-=1      // [expect] 1
print e.a.b         // [expect] 1
print e.a.b *= 2.5  // [expect] 2.5
print e.a.b         // [expect] 2.5
print e.a.b %= 2    // [expect] 0.5
print e.a.b         // [expect] 0.5
print e.a.b /= 2    // [expect] 0.25
print e.a.b         // [expect] 0.25

let f = {"a": [1]}
print f.a[0]+=1     // [expect] 2
print f.a[0]        // [expect] 2
print f.a[0]-=1     // [expect] 1
print f.a[0]        // [expect] 1
print f.a[0] *= 2.5 // [expect] 2.5
print f.a[0]        // [expect] 2.5
print f.a[0] %= 2   // [expect] 0.5
print f.a[0]        // [expect] 0.5
print f.a[0] /= 2   // [expect] 0.25
print f.a[0]        // [expect] 0.25
