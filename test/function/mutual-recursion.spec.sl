fn isEven(n) {
  if (n == 0) ret true;
  ret isOdd(n - 1);
}

fn isOdd(n) {
  if (n == 0) ret false;
  ret isEven(n - 1);
}

print isEven(4) // [Expect] true
print isOdd(3) // [Expect] true
