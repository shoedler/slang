let a = [1,2,3]   // [exit] 2
a[0..2] = [4,5,6] // [expect-error] Compile error at line 2 at '=': Slices can't be assigned to.