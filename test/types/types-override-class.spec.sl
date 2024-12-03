print true is Bool // [expect] true

// This will shadow the native Bool class to some extent.
cls Bool {
  ctor (val) {
    if !(val is typeof(true)) throw "Bool ctor expects a bool" // Can't use 'is Bool' here, because that's now this class.
    this.val = val
  }

  fn to_str -> "bool=" + this.val
}

print true is Bool // [expect] false

// Won't shadow the native Bool class, because that's what's getting used internally.
print true // [expect] true
print typeof(true) == Bool // [expect] false

print Bool(false) // [expect] bool=false
print Bool(true) // [expect] bool=true