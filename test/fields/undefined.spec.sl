cls Foo {}
let foo = Foo()

foo.bar // [ExpectRuntimeError] Property 'bar' does not exist on type Instance.
        // [ExpectRuntimeError] at line 4 at the toplevel
