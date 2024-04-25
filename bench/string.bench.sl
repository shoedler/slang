import Perf

let start = Perf.now()

for let i = 0; i < 10000; i++; {
    let a = 1000.to_str()
    let b = 1.1245.to_str()
    let c = "hello".to_str()
    let d = true.to_str()
    let e = nil.to_str()
    let f = [1, 2, 3].to_str()
    let g = {"a": 1, "b": 2}.to_str()
    let h = a + b + c + d + e + f + g
}

print "elapsed: " + (Perf.now() - start).to_str() + "ms"
