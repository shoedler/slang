import File
print File // [expect] <Instance of Module>

// Does join 2 strings with a path separator
print File.join_path("a", "b") // [expect] a\b

// Does not trim prepended separators on a (even if it's the wrong separator for the platform)
print File.join_path("/a", "b") // [expect] \a\b

// Does not trim appended separators on b (even if it's the wrong separator for the platform)
print File.join_path("a", "b/") // [expect] a\b\

// Does trim too many separators between a and b
print File.join_path("/a/", "/b/") // [expect] \a\b\

// Returns a single separator if both a and b are empty
print File.join_path("", "") // [expect] \

// Returns a single separator if a is empty
print File.join_path("", "b") // [expect] \b

// Returns a single separator if b is empty
print File.join_path("a", "") // [expect] a\

// Fuzzy tests
print File.join_path("a\\", "/b") // [expect] a\b
print File.join_path("/\\a\\b\\//", "c") // [expect] \a\b\c
print File.join_path("/\\a\\b\\//", "/\\a\\b\\//") // [expect] \a\b\a\b\
