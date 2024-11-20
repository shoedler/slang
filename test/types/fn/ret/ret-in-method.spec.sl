cls Foo {
  fn method() {
    ret "ok"
    print "bad"
  }
}

print Foo().method() // [Expect] ok
