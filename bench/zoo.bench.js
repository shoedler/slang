class Zoo {
  constructor() {
    this.aardvark = 1;
    this.baboon = 1;
    this.cat = 1;
    this.donkey = 1;
    this.elephant = 1;
    this.fox = 1;
  }
  ant() {
    return this.aardvark;
  }
  banana() {
    return this.baboon;
  }
  tuna() {
    return this.cat;
  }
  hay() {
    return this.donkey;
  }
  grass() {
    return this.elephant;
  }
  mouse() {
    return this.fox;
  }
}

let zoo = new Zoo();
let duration = 5;
let sum = 0;
let batches = 0;

let start = Date.now();
let i = 0;
while (Date.now() < start + duration * 1000) {
  while (i < 10000) {
    sum =
      sum +
      zoo.ant() +
      zoo.banana() +
      zoo.tuna() +
      zoo.hay() +
      zoo.grass() +
      zoo.mouse();
    i = i + 1;
  }
  batches = batches + 1;
  i = 0;
}

console.log(batches); // [Throughput]
console.log(sum); // [Value]
console.log(duration); // [DurationInSecs]
