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
let other = Other() // [expect-error] Uncaught error: Undefined variable 'Other'.
                    // [expect-error]     20 | let other = Other()
                    // [expect-error]                      ~~~~~
                    // [expect-error]   at line 20 at the toplevel of module "main"
                    // [expect] 2
