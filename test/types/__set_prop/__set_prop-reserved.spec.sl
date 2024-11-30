// [exit] 2
cls X { ctor {} }

print X().__name = 123 // [expect-error] Compile error at line 4 at '__name': Cannot set reserved property '__name'.