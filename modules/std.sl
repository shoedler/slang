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