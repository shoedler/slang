fn x() {  // [exit] 2
  ret 1 2 // [expect-error] Parser error at line 2 at '2': Expecting expression, or newline, '}' or some other statement after return.
}         // [expect-error]      2 |   ret 1 2
          // [expect-error]                  ~