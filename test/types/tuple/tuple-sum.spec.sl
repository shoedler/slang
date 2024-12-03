// Works for ints
print (1,2,3).sum() // [expect] 6
print (-1,1).sum() // [expect] 0

// Works for floats
print (1.5,2.5,3.5).sum() // [expect] 7.5
print (-1.5,1.5).sum() // [expect] 0

// Works for strings
print ("a","b","c").sum() // [expect] abc

// ... that's because these all have a SP_METHOD_ADD method. 

// Should work for custom types too:
cls Vec {
  ctor(x, y) { this.pos = (x, y) }
  fn add(other) {
    const (x, y) = this.pos
    const (ox, oy) = other.pos
    ret Vec(x + ox, y + oy)
  }
}

print (Vec(1,1),Vec(2,2),Vec(3,3)).sum() // [expect] <Instance of Vec>
print (Vec(1,1),Vec(2,2),Vec(3,3)).sum().pos // [expect] (6, 6)

// First type must have a SP_METHOD_ADD method
print try (nil,2,3).sum() else error // [expect] Method "Nil.add" does not exist.

// Works for empty lists
print (,).sum() // [expect] nil

// Works for lists with a single element
print (1,).sum() // [expect] 1
print (1.5,).sum() // [expect] 1.5
print ("a",).sum() // [expect] a

// Propagates errors
print try (1,2,nil).sum() else error // [expect] Incompatible types for binary operand '+': Int + Nil.
