// [exit] 2
let f 
const global = 1
 
{
  fn fun() {
    global++ // [expect-error] Compile error at line 7 at 'global': Can't reassign a constant.
  }          // [expect-error] Compile error at line 14 at end: Expecting '}' after block.
             // [expect-error] Compile error at line 14 at end: Expecting '}' after block.
  f = fun
}

f()
