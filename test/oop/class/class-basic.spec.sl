cls Scone {
  fn topping(first, second) {
    print "scone with " + first + " and " + second
    ret 1
  }
}

let scone = Scone()
let res = scone.topping("berries", "cream") // [expect] scone with berries and cream
print res // [expect] 1
print scone.topping // [expect] <Fn topping>