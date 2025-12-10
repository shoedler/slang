let m = "[...#.]"
print "m = " + m
print "m.len = " + m.len

let m_reversed = ""
for let i = m.len - 2; i >= 1; i--; {
  let ch = m[i]
  print "i = " + i + ", ch = " + ch
  if ch == "#" {
    m_reversed = m_reversed + "1"
  } else if ch == "." {
    m_reversed = m_reversed + "0"
  } else {
    m_reversed = m_reversed + ch
  }
}

print "m_reversed = " + m_reversed
