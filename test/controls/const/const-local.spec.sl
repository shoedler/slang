// [exit] 2
const r = 1
{
  r += 1 // [expect-error] Resolver error at line 4: Cannot assign to constant variable 'r'.
}        // [expect-error]      4 |   r += 1
         // [expect-error]            ~