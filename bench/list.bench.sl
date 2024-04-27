import Perf

let start = Perf.now()
let list = []
for let i = 0; i < 1000000; i++; {
  list.push(i)
}

let sum = 0
for let i = 0; i < list.len; i++; {
  sum += list[i]
}
print sum
print "elapsed: " + (Perf.now() - start).to_str() + "s"