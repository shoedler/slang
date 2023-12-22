fn returnArg(arg) {
  ret arg;
}

fn returnFunCallWithArg(func, arg) {
  ret returnArg(func)(arg);
}

fn printArg(arg) {
  print arg
}

returnFunCallWithArg(printArg, "hello world") // [Expect] hello world
