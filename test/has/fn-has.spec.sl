print (fn -> 1).has("__name") // [Expect] true
print (fn -> 1).has("__doc") // [Expect] false

// Does not work with callables or other non-string types.
print try (fn -> 1).has(fn (x) -> x == "__name") else error // [Expect] Expected argument 0 of type Str but got Fn.
print try (fn -> 1).has(true) else error                    // [Expect] Expected argument 0 of type Str but got Bool.