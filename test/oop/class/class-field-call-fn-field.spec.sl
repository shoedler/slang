cls Foo {}

fn bar(a, b) {
  print "bar"
  print a
  print b
}

let foo = Foo()
foo.bar = bar

foo.bar(1, 2)
// [Expect] bar
// [Expect] 1
// [Expect] 2
