cls Test {
  static fn assert() {
    ret Test();
  }
}

let t = Test.assert()
print t is Test // [Expect] true

print Test.assert // [Expect] <Fn assert>
