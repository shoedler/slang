// [exit] 2
const r = 1
{
  r += 1 // [expect-error] Compile error at line 4 at 'r': Can't reassign a constant.
}        // [expect-error] Compile error at line 6 at end: Expecting '}' after block.
