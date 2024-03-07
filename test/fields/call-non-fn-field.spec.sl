cls Foo {}

let foo = Foo()
foo.bar = "not fn"

foo.bar() // [ExpectRuntimeError] Attempted to call non-callable value of type Str.
          // [ExpectRuntimeError] at line 6 at the toplevel