cls A {
  fn say() {
    print "A"
  }
}

cls B : A {
  fn get_closure() {
    fn closure() {
      base.say()
    }
    ret closure
  }

  fn say() {
    print "B"
  }
}

cls C : B {
  fn say() {
    print "C"
  }
}

C().get_closure()() // [expect] A
