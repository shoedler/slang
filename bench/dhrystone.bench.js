const VERSION = "1.1";
const LOOPS = 50000;

const [Ident1, Ident2, Ident3, Ident4, Ident5] = [1, 2, 3, 4, 5];

class Record {
  constructor(
    PtrComp = null,
    Discr = 0,
    EnumComp = 0,
    IntComp = 0,
    StringComp = 0
  ) {
    this.PtrComp = PtrComp;
    this.Discr = Discr;
    this.EnumComp = EnumComp;
    this.IntComp = IntComp;
    this.StringComp = StringComp;
  }

  copy() {
    return new Record(
      this.PtrComp,
      this.Discr,
      this.EnumComp,
      this.IntComp,
      this.StringComp
    );
  }
}

const TRUE = 1;
const FALSE = 0;

let IntGlob = 0;
let BoolGlob = FALSE;
let Char1Glob = "\0";
let Char2Glob = "\0";
let Array1Glob = new Array(51).fill(0);
let Array2Glob = Array(51)
  .fill()
  .map(() => Array1Glob.slice());
let PtrGlb = null;
let PtrGlbNext = null;

function Proc0(loops = LOOPS) {
  let starttime = process.hrtime.bigint();
  for (let i = 0; i < loops; i++) {}
  let nulltime = process.hrtime.bigint() - starttime;

  PtrGlbNext = new Record();
  PtrGlb = new Record();
  PtrGlb.PtrComp = PtrGlbNext;
  PtrGlb.Discr = Ident1;
  PtrGlb.EnumComp = Ident3;
  PtrGlb.IntComp = 40;
  PtrGlb.StringComp = "DHRYSTONE PROGRAM, SOME STRING";
  let String1Loc = "DHRYSTONE PROGRAM, 1'ST STRING";
  Array2Glob[8][7] = 10;

  starttime = process.hrtime.bigint();

  for (let i = 0; i < loops; i++) {
    Proc5();
    Proc4();
    let IntLoc1 = 2;
    let IntLoc2 = 3;
    let String2Loc = "DHRYSTONE PROGRAM, 2'ND STRING";
    let EnumLoc = Ident2;
    BoolGlob = !Func2(String1Loc, String2Loc);
    let IntLoc3;
    while (IntLoc1 < IntLoc2) {
      IntLoc3 = 5 * IntLoc1 - IntLoc2;
      IntLoc3 = Proc7(IntLoc1, IntLoc2);
      IntLoc1 = IntLoc1 + 1;
    }
    Proc8(Array1Glob, Array2Glob, IntLoc1, IntLoc3);
    PtrGlb = Proc1(PtrGlb);
    let CharIndex = "A";
    while (CharIndex <= Char2Glob) {
      if (EnumLoc == Func1(CharIndex, "C")) EnumLoc = Proc6(Ident1);
      CharIndex = String.fromCharCode(CharIndex.charCodeAt(0) + 1);
    }
    IntLoc3 = IntLoc2 * IntLoc1;
    IntLoc2 = IntLoc3 / IntLoc1;
    IntLoc2 = 7 * (IntLoc3 - IntLoc2) - IntLoc1;
    IntLoc1 = Proc2(IntLoc1);
  }

  let benchtime = process.hrtime.bigint() - starttime - nulltime;
  benchtime = Number(benchtime) / 1_000_000_000;
  let loopsPerBenchtime = benchtime === 0 ? 0 : loops / benchtime;
  return [benchtime / 1000, loopsPerBenchtime];
}

function Proc1(PtrParIn) {
  PtrParIn.PtrComp = PtrGlb.copy();
  let NextRecord = PtrParIn.PtrComp;
  PtrParIn.IntComp = 5;
  NextRecord.IntComp = PtrParIn.IntComp;
  NextRecord.PtrComp = PtrParIn.PtrComp;
  NextRecord.PtrComp = Proc3(NextRecord.PtrComp);
  if (NextRecord.Discr == Ident1) {
    NextRecord.IntComp = 6;
    NextRecord.EnumComp = Proc6(PtrParIn.EnumComp);
    NextRecord.PtrComp = PtrGlb.PtrComp;
    NextRecord.IntComp = Proc7(NextRecord.IntComp, 10);
  } else {
    PtrParIn = NextRecord.copy();
  }
  NextRecord.PtrComp = null;
  return PtrParIn;
}

function Proc2(IntParIO) {
  let IntLoc = IntParIO + 10;
  let EnumLoc;
  while (true) {
    if (Char1Glob == "A") {
      IntLoc = IntLoc - 1;
      IntParIO = IntLoc - IntGlob;
      EnumLoc = Ident1;
    }
    if (EnumLoc == Ident1) break;
  }
  return IntParIO;
}

function Proc3(PtrParOut) {
  if (PtrGlb != null) PtrParOut = PtrGlb.PtrComp;
  else IntGlob = 100;
  PtrGlb.IntComp = Proc7(10, IntGlob);
  return PtrParOut;
}

function Proc4() {
  let BoolLoc = Char1Glob == "A";
  BoolLoc = BoolLoc || BoolGlob;
  Char2Glob = "B";
}

function Proc5() {
  Char1Glob = "A";
  BoolGlob = FALSE;
}

function Proc6(EnumParIn) {
  let EnumParOut = EnumParIn;
  if (!Func3(EnumParIn)) EnumParOut = Ident4;
  if (EnumParIn == Ident1) EnumParOut = Ident1;
  else if (EnumParIn == Ident2) EnumParOut = IntGlob > 100 ? Ident1 : Ident4;
  else if (EnumParIn == Ident3) EnumParOut = Ident2;
  else if (EnumParIn == Ident4);
  else if (EnumParIn == Ident5) EnumParOut = Ident3;
  return EnumParOut;
}

function Proc7(IntParI1, IntParI2) {
  let IntLoc = IntParI1 + 2;
  let IntParOut = IntParI2 + IntLoc;
  return IntParOut;
}

function Proc8(Array1Par, Array2Par, IntParI1, IntParI2) {
  let IntLoc = IntParI1 + 5;
  Array1Par[IntLoc] = IntParI2;
  Array1Par[IntLoc + 1] = Array1Par[IntLoc];
  Array1Par[IntLoc + 30] = IntLoc;
  for (let IntIndex = IntLoc; IntIndex < IntLoc + 2; IntIndex++)
    Array2Par[IntLoc][IntIndex] = IntLoc;
  Array2Par[IntLoc][IntLoc - 1]++;
  Array2Par[IntLoc + 20][IntLoc] = Array1Par[IntLoc];
  IntGlob = 5;
}

function Func1(CharPar1, CharPar2) {
  let CharLoc1 = CharPar1;
  let CharLoc2 = CharLoc1;
  return CharLoc2 != CharPar2 ? Ident1 : Ident2;
}

function Func2(StrParI1, StrParI2) {
  let IntLoc = 1;
  let CharLoc;
  while (IntLoc <= 1) {
    if (Func1(StrParI1[IntLoc], StrParI2[IntLoc + 1]) == Ident1) {
      CharLoc = "A";
      IntLoc++;
    }
  }
  if (CharLoc >= "W" && CharLoc <= "Z") IntLoc = 7;
  if (CharLoc == "X") return TRUE;
  if (StrParI1 > StrParI2) {
    IntLoc += 7;
    return TRUE;
  }
  return FALSE;
}

function Func3(EnumParIn) {
  let EnumLoc = EnumParIn;
  return EnumLoc == Ident3;
}

function main(loops = LOOPS) {
  let [benchtime, stones] = Proc0(loops);
  console.log(
    `JsDhrystone(${VERSION}) time for ${loops} passes = ${benchtime}`
  );
  console.log(`This machine benchmarks at ${stones} stones/second`);
}

main(LOOPS);
