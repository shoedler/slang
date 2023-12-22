cls A {
  fn say() {
    print "A"
  }
}

cls B : A {
  fn test() {
    base.say()
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

C().test() // [Expect] A
