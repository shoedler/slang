// [exit] 2
cls Main {
  ctor {
    this.x = 1
  }

  fn define_other_class {
    cls Other {
      ctor {
        this.y = 2
      }
    }

    let other = Other()
    print other.y
  }
}

let main = Main()
main.define_other_class()
let other = Other() // [expect-error] Resolver error at line 21: Undefined variable 'Other'.
                    // [expect-error]     21 | let other = Other()
                    // [expect-error]                      ~~~~~
