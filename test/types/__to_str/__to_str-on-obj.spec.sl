print {}.to_str()                   // [expect] {}
print {1:2}.to_str()                // [expect] {1: 2}
print {"a":"b"}.to_str()            // [expect] {a: b}
print {true: false}.to_str()        // [expect] {true: false}
print {nil: nil}.to_str()           // [expect] {nil: nil}
print {"a": {1: fn -> 1 }}.to_str() // [expect] {a: {1: <Fn $anon_fn$>}}
print {[]: [fn -> 1]}.to_str()      // [expect] {[]: [<Fn $anon_fn$>]}

print {}                   // [expect] {}
print {1:2}                // [expect] {1: 2}
print {"a":"b"}            // [expect] {a: b}
print {true: false}        // [expect] {true: false}
print {nil: nil}           // [expect] {nil: nil}
print {"a": {1: fn -> 1 }} // [expect] {a: {1: <Fn $anon_fn$>}}
print {[]: [fn -> 1]}      // [expect] {[]: [<Fn $anon_fn$>]}