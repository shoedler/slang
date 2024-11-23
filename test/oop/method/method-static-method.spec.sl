cls Test {
  static fn assert() {
    ret Test()
  }
}

let t = Test.assert()
print t is Test // [expect] true

print Test.assert // [expect] <Fn assert>
