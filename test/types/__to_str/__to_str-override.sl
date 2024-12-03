cls ListWithoutStrOverride {
  ctor {
    this.list = [1,2,[true, nil, "lol"]]
  }
}

cls List {
  ctor {
    this.list = [1,2,[true, nil, "lol"]]
  }

  fn to_str {
    ret "List with " + this.list.len + " elements: " + this.list
  }
}

print ListWithoutStrOverride() // [expect] <Instance of ListWithoutStrOverride>

print List() // [expect] List with 3 elements: [1, 2, [true, nil, lol]]