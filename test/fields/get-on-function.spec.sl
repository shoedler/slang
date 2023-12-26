fn foo() {}

foo.bar // [ExpectRuntimeError] Only instances can have properties.
        // [ExpectRuntimeError] at line 3 at the toplevel