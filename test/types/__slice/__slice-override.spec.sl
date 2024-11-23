cls ListWithoutSliceOverride {
  ctor {
    this.list = [1,2,[true, nil, "lol"]]
  }
}

cls List {
  ctor {
    this.list = [1,2,[true, nil, "lol"]]
  }

  fn slice(a,b) {
    ret this.list.slice(a,b)
  }
}

print try ListWithoutSliceOverride().slice(0, 1) else error // [expect] Undefined callable 'slice' in type ListWithoutSliceOverride or any of its parent classes.

const list = List()

print list.slice(0, 2) // [expect] [1, 2]

// Works with the '[]' operator
print list[1..2] // [expect] [2]