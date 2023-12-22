cls Foo {}

let foo = Foo()

print foo.bar = "bar value" // [Expect] bar value
print foo.baz = "baz value" // [Expect] baz value

print foo.bar // [Expect] bar value
print foo.baz // [Expect] baz value
