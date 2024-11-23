print (fn -> 1).has("__name") // [expect] true

// Does not work with callables or other non-string types.
print try (fn -> 1).has(fn (x) -> x == "__name") else error // [expect] Expected argument 0 of type Str but got Fn.
print try (fn -> 1).has(true) else error                    // [expect] Expected argument 0 of type Str but got Bool.