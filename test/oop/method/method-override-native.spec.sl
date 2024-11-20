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
    ret "List with " + this.list.len.to_str() + " elements: " + this.list.to_str()
  }
}

print ListWithoutStrOverride().to_str() // [Expect] <Instance of ListWithoutStrOverride>
print List().to_str() // [Expect] List with 3 elements: [1, 2, [true, nil, lol]]