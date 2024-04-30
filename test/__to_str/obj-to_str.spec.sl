print {}.to_str()                   // [Expect] {}
print {1:2}.to_str()                // [Expect] {1: 2}
print {"a":"b"}.to_str()            // [Expect] {a: b}
print {true: false}.to_str()        // [Expect] {true: false}
print {nil: nil}.to_str()           // [Expect] {nil: nil}
print {"a": {1: fn -> 1 }}.to_str() // [Expect] {a: {1: <Fn (anon)>}}
print {[]: [fn -> 1]}.to_str()      // [Expect] {[]: [<Fn (anon)>]}

print {}                   // [Expect] {}
print {1:2}                // [Expect] {1: 2}
print {"a":"b"}            // [Expect] {a: b}
print {true: false}        // [Expect] {true: false}
print {nil: nil}           // [Expect] {nil: nil}
print {"a": {1: fn -> 1 }} // [Expect] {a: {1: <Fn (anon)>}}
print {[]: [fn -> 1]}      // [Expect] {[]: [<Fn (anon)>]}