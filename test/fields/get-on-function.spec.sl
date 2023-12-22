fn foo() {}

foo.bar // [ExpectRuntimeError] Only instances can have properties.
        // [ExpectRuntimeError] [line 3] in fn toplevel