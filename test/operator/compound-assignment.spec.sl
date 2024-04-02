
let a = [1]

print a[0]+=1      // [Expect] 2
print a[0]         // [Expect] 2
print a[0]-=1      // [Expect] 1
print a[0]         // [Expect] 1
print a[0] *= 2.5  // [Expect] 2.5
print a[0]         // [Expect] 2.5
print a[0] %= 2    // [Expect] 0.5
print a[0]         // [Expect] 0.5
print a[0] /= 2    // [Expect] 0.25
print a[0]         // [Expect] 0.25

let b = 1
print b+=1         // [Expect] 2
print b            // [Expect] 2
print b-=1         // [Expect] 1
print b            // [Expect] 1
print b *= 2.5     // [Expect] 2.5
print b            // [Expect] 2.5
print b %= 2       // [Expect] 0.5
print b            // [Expect] 0.5
print b /= 2       // [Expect] 0.25
print b            // [Expect] 0.25

let c = {"a": 1}
print c.a+=1       // [Expect] 2
print c.a          // [Expect] 2
print c.a-=1       // [Expect] 1
print c.a          // [Expect] 1
print c.a *= 2.5   // [Expect] 2.5
print c.a          // [Expect] 2.5
print c.a %= 2     // [Expect] 0.5
print c.a          // [Expect] 0.5
print c.a /= 2     // [Expect] 0.25
print c.a          // [Expect] 0.25

let d = [[1]]
print d[0][0]+=1      // [Expect] 2
print d[0][0]         // [Expect] 2
print d[0][0]-=1      // [Expect] 1
print d[0][0]         // [Expect] 1
print d[0][0] *= 2.5  // [Expect] 2.5
print d[0][0]         // [Expect] 2.5
print d[0][0] %= 2    // [Expect] 0.5
print d[0][0]         // [Expect] 0.5
print d[0][0] /= 2    // [Expect] 0.25
print d[0][0]         // [Expect] 0.25

let e = {"a": {"b": 1}}
print e.a.b+=1      // [Expect] 2
print e.a.b         // [Expect] 2
print e.a.b-=1      // [Expect] 1
print e.a.b         // [Expect] 1
print e.a.b *= 2.5  // [Expect] 2.5
print e.a.b         // [Expect] 2.5
print e.a.b %= 2    // [Expect] 0.5
print e.a.b         // [Expect] 0.5
print e.a.b /= 2    // [Expect] 0.25
print e.a.b         // [Expect] 0.25

let f = {"a": [1]}
print f.a[0]+=1     // [Expect] 2
print f.a[0]        // [Expect] 2
print f.a[0]-=1     // [Expect] 1
print f.a[0]        // [Expect] 1
print f.a[0] *= 2.5 // [Expect] 2.5
print f.a[0]        // [Expect] 2.5
print f.a[0] %= 2   // [Expect] 0.5
print f.a[0]        // [Expect] 0.5
print f.a[0] /= 2   // [Expect] 0.25
print f.a[0]        // [Expect] 0.25
