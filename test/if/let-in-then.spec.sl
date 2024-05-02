// [Exit] 2
if false let bar // [ExpectError] Compile error at line 2 at 'let': Expecting expression.
if true let foo // [ExpectError] Compile error at line 3 at 'let': Expecting expression.
