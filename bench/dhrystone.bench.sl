import Perf

const VERSION = "1.1"
const LOOPS = 50000

const [Ident1, Ident2, Ident3, Ident4, Ident5] = [1, 2, 3, 4, 5]

cls Record {
  ctor(PtrComp, Discr, EnumComp, IntComp, StringComp) {
    this.PtrComp = PtrComp
    this.Discr = Discr
    this.EnumComp = EnumComp
    this.IntComp = IntComp
    this.StringComp = StringComp
  }

  static fn empty -> Record(nil, 0, 0, 0, 0)
  
  fn copy -> Record(this.PtrComp, this.Discr, this.EnumComp, this.IntComp, this.StringComp)
}

const TRUE = 1
const FALSE = 0

let IntGlob = 0
let BoolGlob = FALSE
let Char1Glob = "\0"
let Char2Glob = "\0"
let Array1Glob = Seq(51).map(fn -> 0)
let Array2Glob = Seq(51).map(fn -> Array1Glob[..])
let PtrGlb = nil
let PtrGlbNext = nil

fn Proc0(loops) {
  let starttime = Perf.now()
  for let i = 0; i < loops; ++i; {}
  const nulltime = Perf.now() - starttime

  PtrGlbNext = Record.empty()
  PtrGlb = Record.empty()
  PtrGlb.PtrComp = PtrGlbNext
  PtrGlb.Discr = Ident1
  PtrGlb.EnumComp = Ident3
  PtrGlb.IntComp = 40
  PtrGlb.StringComp = "DHRYSTONE PROGRAM, SOME STRING"
  let String1Loc = "DHRYSTONE PROGRAM, 1'ST STRING"
  Array2Glob[8][7] = 10

  starttime = Perf.now()

  for let i = 0; i < loops; ++i; {
    Proc5()
    Proc4()
    let IntLoc1 = 2
    let IntLoc2 = 3
    let String2Loc = "DHRYSTONE PROGRAM, 2'ND STRING"
    let EnumLoc = Ident2
    BoolGlob = !Func2(String1Loc, String2Loc)
    let IntLoc3
    while IntLoc1 < IntLoc2 {
      IntLoc3 = 5 * IntLoc1 - IntLoc2
      IntLoc3 = Proc7(IntLoc1, IntLoc2)
      IntLoc1 = IntLoc1 + 1
    }
    Proc8(Array1Glob, Array2Glob, IntLoc1, IntLoc3)
    PtrGlb = Proc1(PtrGlb)
    let CharIndex = "A"
    while CharIndex <= Char2Glob {
      if EnumLoc == Func1(CharIndex, "C") EnumLoc = Proc6(Ident1)
      CharIndex = Str.from_ascii(CharIndex.ascii_at(0) + 1)
    }
    IntLoc3 = IntLoc2 * IntLoc1
    IntLoc2 = IntLoc3 / IntLoc1
    IntLoc2 = 7 * (IntLoc3 - IntLoc2) - IntLoc1
    IntLoc1 = Proc2(IntLoc1)
  }

  const benchtime = Perf.now() - starttime - nulltime
  const loopsPerBenchtime = benchtime == 0 ? 0 : loops / benchtime
  ret [benchtime, loopsPerBenchtime]
}

fn Proc1(PtrParIn) {
  PtrParIn.PtrComp = PtrGlb.copy()
  let NextRecord = PtrParIn.PtrComp
  PtrParIn.IntComp = 5
  NextRecord.IntComp = PtrParIn.IntComp
  NextRecord.PtrComp = PtrParIn.PtrComp
  NextRecord.PtrComp = Proc3(NextRecord.PtrComp)
  if NextRecord.Discr == Ident1 {
    NextRecord.IntComp = 6
    NextRecord.EnumComp = Proc6(PtrParIn.EnumComp)
    NextRecord.PtrComp = PtrGlb.PtrComp
    NextRecord.IntComp = Proc7(NextRecord.IntComp, 10)
  } else {
    PtrParIn = NextRecord.copy()
  }
  NextRecord.PtrComp = nil
  ret PtrParIn
}

fn Proc2(IntParIO) {
  let IntLoc = IntParIO + 10
  let EnumLoc
  while true {
    if Char1Glob == "A" {
      IntLoc = IntLoc - 1
      IntParIO = IntLoc - IntGlob
      EnumLoc = Ident1
    }
    if EnumLoc == Ident1 break
  }
  ret IntParIO
}

fn Proc3(PtrParOut) {
  if PtrGlb != nil PtrParOut = PtrGlb.PtrComp
  else IntGlob = 100
  PtrGlb.IntComp = Proc7(10, IntGlob)
  ret PtrParOut
}

fn Proc4 {
  let BoolLoc = Char1Glob == "A"
  BoolLoc = BoolLoc or BoolGlob
  Char2Glob = "B"
}

fn Proc5 {
  Char1Glob = "A"
  BoolGlob = FALSE
}

fn Proc6(EnumParIn) {
  let EnumParOut = EnumParIn
  if !Func3(EnumParIn) EnumParOut = Ident4
  if EnumParIn == Ident1 EnumParOut = Ident1
  else if EnumParIn == Ident2 {EnumParOut = IntGlob > 100 ? Ident1 : Ident4}
  else if EnumParIn == Ident3 {EnumParOut = Ident2}
  else if EnumParIn == Ident4 {}
  else if EnumParIn == Ident5 {EnumParOut = Ident3}
  ret EnumParOut
}

fn Proc7(IntParI1, IntParI2) {
  let IntLoc = IntParI1 + 2
  let IntParOut = IntParI2 + IntLoc
  ret IntParOut
}

fn Proc8(Array1Par, Array2Par, IntParI1, IntParI2) {
  let IntLoc = IntParI1 + 5
  Array1Par[IntLoc] = IntParI2
  Array1Par[IntLoc + 1] = Array1Par[IntLoc]
  Array1Par[IntLoc + 30] = IntLoc
  for let IntIndex = IntLoc; IntIndex < IntLoc + 2; ++IntIndex;
    Array2Par[IntLoc][IntIndex] = IntLoc
  ++Array2Par[IntLoc][IntLoc - 1]
  Array2Par[IntLoc + 20][IntLoc] = Array1Par[IntLoc]
  IntGlob = 5
}

fn Func1(CharPar1, CharPar2) {
  let CharLoc1 = CharPar1
  let CharLoc2 = CharLoc1
  ret CharLoc2 != CharPar2 ? Ident1 : Ident2
}

fn Func2(StrParI1, StrParI2) {
  let IntLoc = 1
  let CharLoc
  while IntLoc <= 1 {
    if Func1(StrParI1[IntLoc], StrParI2[IntLoc + 1]) == Ident1 {
      CharLoc = "A"
      ++IntLoc
    }
  }
  if CharLoc >= "W" and CharLoc <= "Z" { IntLoc = 7 }
  if CharLoc == "X" ret TRUE
  if StrParI1 > StrParI2 {
    IntLoc += 7
    ret TRUE
  }
  ret FALSE
}

fn Func3(EnumParIn) {
  let EnumLoc = EnumParIn
  ret EnumLoc == Ident3
}

fn main(loops, standalone) {
  let [benchtime, stones] = Proc0(loops)
  if standalone {
    print "SlDhrystone("+VERSION+") time for "+loops+" passes = "+benchtime
    print "This machine benchmarks at "+stones+" stones/second"
  }
}

let start = Perf.now()
main(LOOPS, false)
print "elapsed: " + (Perf.now() - start) + "s"

