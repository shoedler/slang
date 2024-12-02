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
maker.brew() // [expect] Enjoy your cup of coffee and chicory
maker.brew() // [expect] Enjoy your cup of nil
