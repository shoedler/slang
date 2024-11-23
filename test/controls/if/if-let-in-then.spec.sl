// [exit] 2
if false let bar // [expect-error] Compile error at line 2 at 'let': Expecting expression.
if true let foo // [expect-error] Compile error at line 3 at 'let': Expecting expression.
