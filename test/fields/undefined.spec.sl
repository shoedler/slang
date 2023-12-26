cls Foo {}
let foo = Foo()

foo.bar // [ExpectRuntimeError] Undefined property 'bar'.
        // [ExpectRuntimeError] at line 4 at the toplevel
