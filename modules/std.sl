cls TypeErr {
  ctor (expected_type, actual_value) {
    this.expected_type = expected_type
    this.actual_value = actual_value
  }

  fn to_str {
    ret "Expected " + this.expected_type.to_str() + " but got " + typeof(this.actual_value).to_str() + " instead."
  }
}

cls Set {
  ctor(seq) {
    this.data = {}
    for let i = 0; i < seq.len; i++; {
      this.data[seq[i]] = true
    }
  }

  fn values -> this.data.keys()

  fn to_str -> "<Set " + this.values().to_str() + ">"

  fn add(value) {
    this.data[value] = true
  }

  fn remove(value) {
    this.data[value] = nil
  }

  fn has(value) {
    ret this.data[value] == true
  }

  fn to_seq() {
    ret this.data.keys()
  }

  fn union(other) {
    const out = Set()

    const these_vals = this.values()
    const other_vals = other.values()

    const min = these_vals.len < other_vals.len ? these_vals.len : other_vals.len
    const longer = these_vals.len < other_vals.len ? other_vals : these_vals

    for let i = 0; i < min; i++; {
      out.add(these_vals[i])
      out.add(other_vals[i])
    }

    for let i = min; i < longer.len; i++; {
      out.add(longer[i])
    }

    ret out
  }

  fn intersection(other) {
    const out = Set()

    const these_vals = this.values()
    const other_vals = other.values()

    for let i = 0; i < these_vals.len; i++; {
      if other.has(these_vals[i]) {
        out.add(these_vals[i])
      }
    }

    ret out
  }

  fn difference(other) {
    const out = Set()

    const these_vals = this.values()
    const other_vals = other.values()

    for let i = 0; i < these_vals.len; i++; {
      if !other.has(these_vals[i]) {
        out.add(these_vals[i])
      }
    }

    ret out
  }

  // fn symmetric_difference(other) {
  //   let out = Set()

  //   let these_vals = this.values()
  //   let other_vals = other.values()

  //   for let i = 0; i < these_vals.len; i++; {
  //     if !other.has(these_vals[i]) {
  //       out.add(these_vals[i])
  //     }
  //   }

  //   for let i = 0; i < other_vals.len; i++; {
  //     if !this.has(other_vals[i]) {
  //       out.add(other_vals[i])
  //     }
  //   }

  //   ret out
  // }

  fn symmetric_difference(other) {
    const out = Set()

    const these_vals = this.values()
    const other_vals = other.values()

    const shorter = these_vals.len < other_vals.len ? this : other
    const longer = these_vals.len < other_vals.len ? other : this

    const longer_vals = longer.values()
    const shorter_vals = shorter.values()

    for let i = 0; i < shorter_vals.len; i++; {
      if !longer.has(shorter_vals[i]) {
        out.add(shorter_vals[i])
      }
      if !shorter.has(longer_vals[i]) {
        out.add(longer_vals[i])
      }
    }

    for let i = shorter_vals.len; i < longer_vals.len; i++; {
      if !shorter.has(longer_vals[i]) {
        out.add(longer_vals[i])
      }
    }

    ret out
  }
}