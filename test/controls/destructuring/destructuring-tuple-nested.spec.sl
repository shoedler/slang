let (a, b, (c, d))  = (1, 2, (3, 4))
print a // [expect] 1
print b // [expect] 2
print c // [expect] 3
print d // [expect] 4

let (((e))) = (((5,),),)
print e // [expect] 5

let (f, (...g)) = (6, (7, 8))
print f // [expect] 6
print g // [expect] (7, 8)

let (h, (i, ...j)) = (9, (10, 11, 12))
print h // [expect] 9
print i // [expect] 10

let (k, (l, ...m), ...n) = (13, (14, 15, 16), 17, 18)
print k // [expect] 13
print l // [expect] 14
print m // [expect] (15, 16)
print n // [expect] (17, 18)

// Mixed
let (o, [p, q]) = (19, [20, 21])
print o // [expect] 19
print p // [expect] 20
print q // [expect] 21

// Local scope
{
  let (a, b, (c, d))  = (1, 2, (3, 4))
  print a // [expect] 1
  print b // [expect] 2
  print c // [expect] 3
  print d // [expect] 4

  let (((e))) = (((5,),),)
  print e // [expect] 5

  let (f, (...g)) = (6, (7, 8))
  print f // [expect] 6
  print g // [expect] (7, 8)

  let (h, (i, ...j)) = (9, (10, 11, 12))
  print h // [expect] 9
  print i // [expect] 10

  let (k, (l, ...m), ...n) = (13, (14, 15, 16), 17, 18)
  print k // [expect] 13
  print l // [expect] 14
  print m // [expect] (15, 16)
  print n // [expect] (17, 18)

  // Mixed
  let (o, [p, q]) = (19, [20, 21])
  print o // [expect] 19
  print p // [expect] 20
  print q // [expect] 21
}