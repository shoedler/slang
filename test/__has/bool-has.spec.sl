print try (true.has(true)) else error // [Expect] Undefined method 'has' in 'Bool' or any of its parent classes.
print try ("to_str" in true) else error // [Expect] Type Bool does not support the 'in' operator. It must implement 'has'.