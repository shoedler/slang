[nil].each(fn (value) {
  const result = try ("" in value) else error
  if !(result is Str) throw "Expected a "+Str+" error"
  if !result.has("does not support \"has\"") throw "Wrong error message for type " + typeof(value) + " Got: " + result
})

print true // [expect] true
