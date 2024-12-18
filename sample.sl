
import File
import Math

const [regs, prg] = File
  .read(cwd() + "sample.txt")
  .split("\r\n\r\n")

const REGS = {}
regs.split("\r\n").each(fn(line) {
  let [reg, val] = line.split(": ")
  reg = reg.split(" ")[1]
  REGS[reg] = val.ints()[0]
})

let {A, B, C} = REGS
const PRG = prg.ints()

fn get_combo(op) {
  if (op>=0 and op<=3) ret op 
  else if (op==4) ret A 
  else if (op==5) ret B 
  else if (op==6) ret C 
  throw "Reserved"
}


fn dv(b) {
  ret Math.floor(A / Math.pow(2, b))
}

fn interpret() {
  let ip = 0
  let out = []


  while ip < PRG.len {
    const cmd = PRG[ip]
    if ip+1 >= PRG.len throw "Unexpected" 
    const op = PRG[ip+1]

    if (cmd==0) { // adv
      A = dv(get_combo(op))
    } else if (cmd==1) { // bxl
      B = Math.xor(B, op)
    } else if (cmd==2) { // bst
      B = get_combo(op)%8    
    } else if (cmd==3) { // jnz
      if (A != 0) {
        ip = op
        skip
      }
    } else if (cmd==4) { // bxc
      B = Math.xor(B, C)
    } else if (cmd==5) { // out
      out.push((get_combo(op)%8))
    } else if (cmd==6) { // bdv
      B = dv(get_combo(op))
    } else if (cmd==7) { // cdv
      C = dv(get_combo(op))
    } else {
      throw "Reserved"
    }
    ip+=2
  }

  print out.join(",")
}

interpret()

// const p1 = 0s
// log("Part 1:", p1) // Part 1: 7,4,2,0,5,0,5,3,7
// log("Part 2:", p2) // Part 2: 