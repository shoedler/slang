cls Foo {}
let foo = Foo()

foo.bar // [ExpectRuntimeError] Undefined property 'bar'.
        // [ExpectRuntimeError] [line 4] in fn toplevel
