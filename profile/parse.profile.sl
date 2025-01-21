import File
import Perf

const digits = "0123456789"

const prg = File
  .read(cwd() + "/parse.profile.input")

fn run(do_enable) {
  let i = 0
  let res = 0
  let enable = true

  fn is_digit() -> prg[i] in digits

  fn check(str) -> prg[i..i+str.len] == str

  fn match(str) {
    if !check(str) ret false
    i += str.len
    ret true
  }

  fn num() {
    let n = ""
    while (is_digit()) {
      n += prg[i]
      ++i
    }
    ret Int(n)
  }

  for ; i < prg.len; ++i ; {
    if do_enable and match("don't()") enable = false
    if do_enable and match("do()") enable = true
    
    if match("mul(") {
      const a = num()
      if !match(",") skip
      const b = num()
      if !check(")") skip
      if enable 
        res += Int(a) * Int(b)
    }
  }
  ret res
}

const start = Perf.now()
print run(false) // 168539636
print run(true)  // 97529391
print "elapsed: " + Perf.since(start) + "s"
