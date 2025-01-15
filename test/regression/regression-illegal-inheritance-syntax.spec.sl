cls A { }     // [exit] 2
cls B < A {}  // [expect-error] Parser error at line 2 at '<': Expecting '{' before class body.
              // [expect-error]      2 | cls B < A {}
              // [expect-error]                ~