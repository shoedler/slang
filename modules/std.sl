cls Range {
  ctor(min, max) {
    this.min = min
    this.max = max
  }

  fn __iter() {
    let me = this
    fn make_iter(index) {
      let self = me
      let i = index
      fn next()  {
        if i >= self.max {
          ret nil
        }
        let out = i
        i = i + 1
        ret out
      }
      ret next
    }
    ret make_iter(this.min)
  }
}

cls List {
  ctor(count) {
    this.count = count
    this.data = Seq(count)
  }

  fn map(f) {
    if typeof(f).__name != typeof(fn -> nil).__name {
      print "Error: map requires a function as an argument"
      ret nil
    }

    let out = List(this.data.len)
    for let i = 0 ; i < this.data.len ; i = i + 1 ; {
      out.data[i] = f(this.data[i], i)
    }
    ret out
  }

  fn to_str() {
    ret "List of " + this.data.len.to_str() + " elements. Items: " + this.data.to_str()
  }
}

cls Monad {
  ctor(value) {
    this.value = value
  }

  fn bind(f) {
    if this.value == nil {
      ret this
    }
    ret Monad(f(this.value))
  }
}

cls Set {
  ctor() {
    this.data = {}
  }

  fn values() {
    ret this.data.keys()
  }

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
    let out = Set()

    let these_vals = this.values()
    let other_vals = other.values()

    let min = these_vals.len < other_vals.len ? these_vals.len : other_vals.len
    let longer = these_vals.len < other_vals.len ? other_vals : these_vals

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
    let out = Set()

    let these_vals = this.values()
    let other_vals = other.values()

    for let i = 0; i < these_vals.len; i++; {
      if other.has(these_vals[i]) {
        out.add(these_vals[i])
      }
    }

    ret out
  }

  fn difference(other) {
    let out = Set()

    let these_vals = this.values()
    let other_vals = other.values()

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
    let out = Set()

    let these_vals = this.values()
    let other_vals = other.values()

    let shorter = these_vals.len < other_vals.len ? this : other
    let longer = these_vals.len < other_vals.len ? other : this

    let longer_vals = longer.values()
    let shorter_vals = shorter.values()

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

  fn to_str() {
    ret "<Set " + this.values().to_str() + ">"
  }
}