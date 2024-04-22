try {
  123.foo = "value"
}
catch {
  print error // [Expect] Type Int does not support property-set access.
}

try {
  (1.2).foo = "value"
}
catch {
  print error // [Expect] Type Float does not support property-set access.
}
