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
          ret nil;
        }
        let out = i
        i = i + 1
        ret out;
      }
      ret next;
    }
    ret make_iter(this.min);
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
      ret nil;
    }

    let out = List(this.data.len())
    for let i = 0 ; i < this.data.len() ; i = i + 1 ; {
      out.data[i] = f(this.data[i], i)
    }
    ret out;
  }

  fn to_str() {
    ret "List of " + this.data.len().to_str() + " elements. Items: " + this.data.to_str();
  }
}

cls Monad {
  ctor(value) {
    this.value = value
  }

  fn bind(f) {
    if this.value == nil {
      ret this;
    }
    ret Monad(f(this.value));
  }
}
