try {
  0.slice(0, 0)
}
catch {
  print error // [expect] Undefined callable 'slice' in type Int or any of its parent classes.
}

try {
  (1.1).slice(0, 0)
}
catch {
  print error // [expect] Undefined callable 'slice' in type Float or any of its parent classes.
}
