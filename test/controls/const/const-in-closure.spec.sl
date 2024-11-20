// [Exit] 2
let f 
const global = 1
 
{
  fn fun() {
    global++ // [ExpectError] Compile error at line 7 at 'global': Can't reassign a constant.
  }          // [ExpectError] Compile error at line 14 at end: Expecting '}' after block.
             // [ExpectError] Compile error at line 14 at end: Expecting '}' after block.
  f = fun
}

f()
