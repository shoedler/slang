
try { 1 / 0 }
catch {
  print error // [expect] Division by zero.
}

try { 1 / 0 }
catch {
  const error = 123 // This works, because the 'error' var is declared in the scope surrounding the whole try-catch statement. This will effectively shadow the synthetic 'error' var
  print error // [expect] 123
}

