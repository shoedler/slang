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
             // [ExpectRuntimeError] [line 7] in fn "brew"
             // [ExpectRuntimeError] [line 18] in fn toplevel
maker.brew() // [Expect] Enjoy your cup of coffee and chicory
