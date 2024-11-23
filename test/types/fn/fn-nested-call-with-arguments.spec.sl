fn return_arg(arg) {
  ret arg
}

fn return_fn_call_with_arg(func, arg) {
  ret return_arg(func)(arg)
}

fn printArg(arg) {
  print arg
}

return_fn_call_with_arg(printArg, "hello world") // [expect] hello world
