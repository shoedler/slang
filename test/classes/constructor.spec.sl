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
maker.brew() // [ExpectError] Uncaught error: Incompatible types for binary operand +. Left was Str, right was Nil.
             // [ExpectError]      7 |     print "Enjoy your cup of " + this.coffee
             // [ExpectError]                                         ~~~~~~~
             // [ExpectError]   at line 7 in "brew" in module "main"
             // [ExpectError]   at line 21 at the toplevel of module "main"

maker.brew() // [Expect] Enjoy your cup of coffee and chicory
