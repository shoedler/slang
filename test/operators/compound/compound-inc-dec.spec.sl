// Postfix
let a = [1]
print a[0]++ // [expect] 1
print a[0]   // [expect] 2
print a[0]-- // [expect] 2
print a[0]   // [expect] 1

let b = 1
print b++ // [expect] 1
print b   // [expect] 2
print b-- // [expect] 2
print b   // [expect] 1
print b-- // [expect] 1
print b-- // [expect] 0
print b   // [expect] -1

let c = {"a": 1}
print c.a++ // [expect] 1
print c.a   // [expect] 2
print c.a-- // [expect] 2
print c.a   // [expect] 1

let d = [[1]]
print d[0][0]++ // [expect] 1
print d[0][0]   // [expect] 2
print d[0][0]-- // [expect] 2
print d[0][0]   // [expect] 1

let e = {"a": {"b": 1}}
print e.a.b++ // [expect] 1
print e.a.b   // [expect] 2
print e.a.b-- // [expect] 2
print e.a.b   // [expect] 1

let f = {"a": [1]}
print f.a[0]++ // [expect] 1
print f.a[0]   // [expect] 2
print f.a[0]-- // [expect] 2
print f.a[0]   // [expect] 1

// Unary/Prefix
let g = [1]
print ++g[0] // [expect] 2
print g[0]   // [expect] 2
print --g[0] // [expect] 1
print g[0]   // [expect] 1

let h = 1
print ++h // [expect] 2
print h   // [expect] 2
print --h // [expect] 1
print h   // [expect] 1
print --h // [expect] 0
print --h // [expect] -1
print h   // [expect] -1

let i = {"a": 1}
print ++i.a // [expect] 2
print i.a   // [expect] 2
print --i.a // [expect] 1
print i.a   // [expect] 1

let j = [[1]]
print ++j[0][0] // [expect] 2
print j[0][0]   // [expect] 2
print --j[0][0] // [expect] 1
print j[0][0]   // [expect] 1

let k = {"a": {"b": 1}}
print ++k.a.b // [expect] 2
print k.a.b   // [expect] 2
print --k.a.b // [expect] 1
print k.a.b   // [expect] 1

let l = {"a": [1]}
print ++l.a[0] // [expect] 2
print l.a[0]   // [expect] 2
print --l.a[0] // [expect] 1
print l.a[0]   // [expect] 1

