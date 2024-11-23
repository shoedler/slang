fn is_even(n) {
  if (n == 0) ret true
  ret is_odd(n - 1)
}

fn is_odd(n) {
  if (n == 0) ret false
  ret is_even(n - 1)
}

print is_even(4) // [expect] true
print is_odd(3) // [expect] true
