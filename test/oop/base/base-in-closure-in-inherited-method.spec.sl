cls A {
  fn say() {
    print "A"
  }
}

cls B : A {
  fn getClosure() {
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

C().getClosure()() // [expect] A
