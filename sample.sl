import File

const NUM_KEYS = [
  ["7", "8", "9"],
  ["4", "5", "6"],
  ["1", "2", "3"],
  [nil, "0", "A"]
]

const DIR_KEYS = [
  [nil, "^", "A"],
  ["<", "v", ">"],
]

const DIRS = { "^": (0,-1), "v": (0,1), ">": (1,0), "<": (-1,0) }

fn make_pad(pad) {
  const pos = {}
  for let y = 0; y < pad.len; y++; {
    for let x = 0; x < pad[y].len; x++; {
      if try pad[y][x]
        pos[pad[y][x]] = (y, x)
    }
  }
  ret pos
}

fn reps(str, n) {
  let res = ""
  for let i = 0; res.len < n; res += str; {}
  ret res
}

fn unique_perms(str) {
  const perms = {}
  const chars = str.split("")

  fn swap(i, j) {
    const tmp = chars[i]
    chars[i] = chars[j]
    chars[j] = tmp
  }

  fn permute(n) {
    if n == 1 {
      perms[Tuple(chars)] = true
    } else {
      for let i = 0; i < n; i++; {
        permute(n-1)
        swap(n % 2 ? 0 : i, n-1)
      }
    }
  }

  permute(chars.len)
  ret perms
}

throw unique_perms("").keys()

const num_keypad = make_pad(NUM_KEYS)
const dir_keypad = make_pad(DIR_KEYS)

fn calc_presses(seq, depth, dirkey, cur) {
  const keypad = dirkey? dir_keypad : num_keypad
  if seq==nil or seq.len == 0 ret 0
  if cur==nil cur = keypad["A"]

  const (cx, cy) = cur
  const (px, py) = keypad[seq[0]]
  const dx = px - cx
  const dy = py - cy

  let moves = ""
  if dx>0 moves += reps(">", dx)
  else if dx<0 moves += reps("<", -dx)
  if dy>0 moves += reps("v", dy)
  else if dy<0 moves += reps("^", -dy)

  let min_len = 999999999
  if depth!=0 {
    const perm_lens = []
    const perms = unique_perms(moves).keys()
    for let i = 0; i<perms.len; i++; {
      const perm = perms[i]
      let (cx, cy) = cur

      let no_break = true
      for let j = 0; j<perm.len; j++; {
        const move = perm[j]
        const (mdx, mdy) = DIRS[move]
        cx += mdx
        cy += mdy
        if !((cx, cy) in keypad.values()) {
          no_break = false
          break
        }
      }
      if no_break {
        const new_seq = perm.join("") + "A"
        perm_lens.push(calc_presses(new_seq, depth-1, true, nil))
      }
    }
    min_len = perm_lens.fold(99999999, fn(a,b) -> a < b ? a : b)
  }
  else {
    min_len = moves.len + 1
    
    print moves.len
  }
  ret min_len * calc_presses(seq[1..], depth, dirkey, (px, py))
}

import Perf
const now = Perf.now()

let p1 = 0
let p2 = 0

const keypad = File
  .read(cwd() + "sample2.txt")
  .split("\r\n")
  .each(fn(code) {
    const code_num = Int(code[..-1])
    p1 += code_num * calc_presses(code, 2, false, nil)
    // p2 += code_num * calc_presses(code, 25, false, nil)
  })

print(p1)
print(p2)

print (Perf.since(now)) // Seconds 
