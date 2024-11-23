// Single-expression body.
for let c = 0; c < 3;; print c = c + 1
// [expect] 1
// [expect] 2
// [expect] 3

// Block body.
for let a = 0; a < 3; a = a + 1; {
  print a
}
// [expect] 0
// [expect] 1
// [expect] 2

// No clauses.
fn foo() {
  for ;;; ret "done"
}
print foo() // [expect] done

// No variable.
let i = 0
for ; i < 2; i = i + 1; print i
// [expect] 0
// [expect] 1

// No condition.
fn bar() {
  for let i = 0;; i = i + 1; {
    print i
    if i >= 2 ret
  }
}
bar()
// [expect] 0
// [expect] 1
// [expect] 2

// No increment.
for let i = 0; i < 2;; {
  print i
  i = i + 1
}
// [expect] 0
// [expect] 1

// Statement bodies.
fn stat_body_1() {
  for ;;; if true ret 1 else ret 2
}
fn stat_body_2() {
  for ;;; while true ret 1
}
fn stat_body_3() {
  for ;;; for ;;; ret 1
}
print stat_body_1() // [expect] 1
print stat_body_2() // [expect] 1
print stat_body_3() // [expect] 1
