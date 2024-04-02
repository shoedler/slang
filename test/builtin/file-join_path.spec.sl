import File
print File // [Expect] <Instance of Module>

// Does join 2 strings with a path separator
print File.join_path("a", "b") // [Expect] a\b

// Does not trim prepended separators on a (even if it's the wrong separator for the platform)
print File.join_path("/a", "b") // [Expect] \a\b

// Does not trim appended separators on b (even if it's the wrong separator for the platform)
print File.join_path("a", "b/") // [Expect] a\b\

// Does trim too many separators between a and b
print File.join_path("/a/", "/b/") // [Expect] \a\b\

// Returns a single separator if both a and b are empty
print File.join_path("", "") // [Expect] \

// Returns a single separator if a is empty
print File.join_path("", "b") // [Expect] \b

// Returns a single separator if b is empty
print File.join_path("a", "") // [Expect] a\

// Fuzzy tests
print File.join_path("a\\", "/b") // [Expect] a\b
print File.join_path("/\\a\\b\\//", "c") // [Expect] \a\b\c
print File.join_path("/\\a\\b\\//", "/\\a\\b\\//") // [Expect] \a\b\a\b\
