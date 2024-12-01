// [exit] 3
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
maker.brew() // [expect-error] Uncaught error: Incompatible types for binary operand +. Left was Str, right was Nil.
             // [expect-error]      8 |     print "Enjoy your cup of " + this.coffee
             // [expect-error]                                         ~~~~~~~~~~~~~
             // [expect-error]   at line 8 in "brew" in module "main"
             // [expect-error]   at line 22 at the toplevel of module "main"

maker.brew() // [expect] Enjoy your cup of coffee and chicory
