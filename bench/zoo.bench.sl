import Perf

cls Zoo {
  ctor {
    this.aardvark = 1
    this.baboon   = 1
    this.cat      = 1
    this.donkey   = 1
    this.elephant = 1
    this.fox      = 1
  }
  fn ant    -> this.aardvark
  fn banana -> this.baboon
  fn tuna   -> this.cat
  fn hay    -> this.donkey
  fn grass  -> this.elephant
  fn mouse  -> this.fox
}

let zoo = Zoo()

let start = Perf.now()

for let k = 0; k < 5; k++; {
  let sum = 0
  for let j = 0; j < 30; j++; {
    let i = 0
    while i < 10000 {
      sum = sum + zoo.ant()
                + zoo.banana()
                + zoo.tuna()
                + zoo.hay()
                + zoo.grass()
                + zoo.mouse()
      i++
    }
  }
  print sum
}

print "elapsed: " + (Perf.now() - start).to_str() + " ms"
