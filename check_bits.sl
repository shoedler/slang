import Math

let configs = [[0,2,3,4], [2,3], [0,4], [0,1,2], [1,2,3,4]]

for let i = 0; i < configs.len; i++; {
  let config = configs[i]
  let bit_val = 0
  for let j = 0; j < config.len; j++; {
    bit_val = bit_val + Math.shl(1, config[j])
  }
  
  // Also show binary
  let binary = ""
  for let b = 0; b < 5; b++; {
    if Math.band(bit_val, Math.shl(1, b)) != 0 {
      binary = "1" + binary
    } else {
      binary = "0" + binary
    }
  }
  
  print "Config " + config + " = " + bit_val + " (binary: " + binary + ")"
}

print ""
print "Target is state 2, which in binary (5 bits) is: 00010"
print "Starting from state 0 (00000):"
print "  No single button gets us to 00010"
print ""
print "Possible XOR combinations:"
for let i = 0; i < configs.len; i++; {
  for let j = 0; j < configs.len; j++; {
    let bit1 = 0
    for let k = 0; k < configs[i].len; k++; {
      bit1 = bit1 + Math.shl(1, configs[i][k])
    }
    let bit2 = 0
    for let k = 0; k < configs[j].len; k++; {
      bit2 = bit2 + Math.shl(1, configs[j][k])
    }
    let result = Math.xor(bit1, bit2)
    if result == 2 {
      print "  " + configs[i] + " XOR " + configs[j] + " = " + result
    }
  }
}
