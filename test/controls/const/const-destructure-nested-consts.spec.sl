const x = 1
const p = [x,2,3]

let [a,b,c] = p

a++ 
print a // [expect] 2
print x // [expect] 1

// You can make a constant reference to a array, but not a constant array
const q = [1,2,3]
const r = [q,2,3]

let [d,e,f] = r

d[0]++ // ... still works
print d[0] // [expect] 2
print q[0] // [expect] 2
