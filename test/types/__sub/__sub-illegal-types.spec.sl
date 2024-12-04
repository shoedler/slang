cls Class{}

[Class, fn -> 1, true, nil, "", {}, [], (,)].each(fn (value) {
  const result = try (value - 1) else error
  if !(result is Str) throw "Expected a "+Str+" error"
  if !result.has("does not support \"sub\"") throw "Wrong error message for type " + typeof(value) + " Got: " + result
})

print true // [expect] true
