cls Class{}
fn function -> 1
let bool = true
let nil_ = nil
let num = 1
let obj = {1: 1}

let types = [Class, function, bool, num, nil_, obj]

types.each(fn (type) {
  let result = try (type[..]) else error
  if !result is Str throw "Expected a string error"
  if !result.has("does not support slicing. It must implement 'slice'") throw "Expected a slice error"
})