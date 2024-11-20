print try (1.has(1)) else error // [Expect] Undefined callable 'has' in type Int or any of its parent classes.
print try ("to_str" in 1) else error // [Expect] Type Int does not support the 'in' operator. It must implement 'has'.
print try ((1.1).has(1)) else error // [Expect] Undefined callable 'has' in type Float or any of its parent classes.
print try ("to_str" in 1.1) else error // [Expect] Type Float does not support the 'in' operator. It must implement 'has'.