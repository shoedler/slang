print try (nil.has(nil)) else error // [Expect] Undefined method 'has' in 'Nil' or any of its parent classes.
print try ("to_str" in nil) else error // [Expect] Type Nil does not support the 'in' operator. It must implement 'has'.