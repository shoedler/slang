// [exit] 2
let f 
const global = 1
 
{
  fn fun() {
    global++ // [expect-error] Resolver error at line 7: Cannot assign to constant variable 'global'.
  }          // [expect-error]      7 |     global++
             // [expect-error]              ~~~~~~
  f = fun
}

f()
