
let a = [1]
print a[0]++ // [Expect] 2
print a[0]   // [Expect] 2
print a[0]-- // [Expect] 1
print a[0]   // [Expect] 1

let b = 1
print b++ // [Expect] 2
print b   // [Expect] 2
print b-- // [Expect] 1
print b   // [Expect] 1
print b-- // [Expect] 0
print b-- // [Expect] -1
print b   // [Expect] -1

let c = {"a": 1}
print c.a++ // [Expect] 2
print c.a   // [Expect] 2
print c.a-- // [Expect] 1
print c.a   // [Expect] 1

let d = [[1]]
print d[0][0]++ // [Expect] 2
print d[0][0]   // [Expect] 2
print d[0][0]-- // [Expect] 1
print d[0][0]   // [Expect] 1

let e = {"a": {"b": 1}}
print e.a.b++ // [Expect] 2
print e.a.b   // [Expect] 2
print e.a.b-- // [Expect] 1
print e.a.b   // [Expect] 1

let f = {"a": [1]}
print f.a[0]++ // [Expect] 2
print f.a[0]   // [Expect] 2
print f.a[0]-- // [Expect] 1
print f.a[0]   // [Expect] 1

