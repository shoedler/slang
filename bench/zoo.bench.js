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

const start = process.hrtime.bigint();

for (let k = 0; k < 5; k++) {
  let sum = 0;
  for (let j = 0; j < 30; j++) {
    let i = 0;
    while (i < 10000) {
      sum =
        sum +
        zoo.ant() +
        zoo.banana() +
        zoo.tuna() +
        zoo.hay() +
        zoo.grass() +
        zoo.mouse();
      i++;
    }
  }
  console.log(sum);
}

console.log(
  `elapsed: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(
    5
  )}s`
);
