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
let duration = 5
let sum = 0
let batches = 0

let start = clock()
let i = 0
while clock() < start + duration {
  while i < 10000 {
    sum = sum + zoo.ant()
            + zoo.banana()
            + zoo.tuna()
            + zoo.hay()
            + zoo.grass()
            + zoo.mouse()
    i = i + 1
  }
  batches = batches + 1
  i = 0
}

// [ThroughputBenchmark] Zoo
print batches  // [Throughput]
print sum      // [Value]
print duration // [DurationInSecs]
