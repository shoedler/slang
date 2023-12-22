cls Foo {}

let foo = Foo(1, 2, 3) // [ExpectRuntimeError] Expected 0 arguments but got 3.
                       // [ExpectRuntimeError] [line 3] in fn toplevel