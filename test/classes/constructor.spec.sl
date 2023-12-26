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
maker.brew() // [ExpectRuntimeError] Operands must be two numbers or two strings.
             // [ExpectRuntimeError] at line 7 in "brew"
             // [ExpectRuntimeError] at line 18 at the toplevel
maker.brew() // [Expect] Enjoy your cup of coffee and chicory
