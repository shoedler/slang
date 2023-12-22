fn outer {
  let x = "value"
  fn middle {
    fn inner { 
      print x
    }

    print "create inner closure"
    ret inner;
  }

  print "return from outer"
  ret middle;
}

let mid = outer() // [Expect] return from outer
let in = mid()    // [Expect] create inner closure
in()              // [Expect] value
