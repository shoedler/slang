cls HasItAll {
  fn has (x) -> true
}

const has_it_all = HasItAll()

print has_it_all.has("does not exist") // [expect] true

// Works with the 'in' operator
print "gugu" in has_it_all             // [expect] true

