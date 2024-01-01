cls A {
  fn method(arg) {
    print "A.method(" + arg + ")"
  }
}

cls B : A {
  fn getClosure() {
    ret base.method;
  }

  fn method(arg) {
    print "B.method(" + arg + ")"
  }
}


let closure = B().getClosure()
closure("arg") // [Expect] A.method(arg)
