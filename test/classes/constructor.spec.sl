cls CoffeeMaker {
  ctor(coffee) {
    this.coffee = coffee
  }

  fn brew {
    print "Enjoy your cup of " + this.coffee

    // No reusing the grounds!
    this.coffee = nil
  }
}

let maker = CoffeeMaker("coffee and chicory")
maker.brew() // [ExpectRuntimeError] Operands must be two numbers or two strings. Left was Str, right was Nil.
             // [ExpectRuntimeError] at line 7 in "brew" in module "main"
             // [ExpectRuntimeError] at line 18 at the toplevel of module "main"
maker.brew() // [Expect] Enjoy your cup of coffee and chicory
