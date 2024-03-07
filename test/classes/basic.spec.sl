cls Scone {
  fn topping(first, second) {
    print "scone with " + first + " and " + second
    ret 1;
  }
}

let scone = Scone()
let res = scone.topping("berries", "cream") // [Expect] scone with berries and cream
print res // [Expect] 1
print scone.topping // [Expect] <Fn topping>