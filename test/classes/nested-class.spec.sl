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
let other = Other() // [ExpectRuntimeError] Uncaught error: Undefined variable 'Other'.
                    // [ExpectRuntimeError] at line 20 at the toplevel of module "main"
                    // [Expect] 2
