cls Class{}

[Class, fn -> 1, true, nil, 1, 1.5, {}].each(fn (value) {
  const result = try (value[..]) else error
  if !(result is Str) throw "Expected a "+Str+" error"
  if !result.has("does not support \"slice\"") throw "Wrong error message for type " + typeof(value) + " Got: " + result
})

print true // [expect] true
