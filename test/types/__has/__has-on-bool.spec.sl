print try (true.has(true)) else error // [expect] Type Bool does not support "has".
print try ("to_str" in true) else error // [expect] Type Bool does not support "has".