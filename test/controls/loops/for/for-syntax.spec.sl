// Single-expression body.
for let c = 0; c < 3;; print c = c + 1
// [Expect] 1
// [Expect] 2
// [Expect] 3

// Block body.
for let a = 0; a < 3; a = a + 1; {
  print a
}
// [Expect] 0
// [Expect] 1
// [Expect] 2

// No clauses.
fn foo() {
  for ;;; ret "done"
}
print foo() // [Expect] done

// No variable.
let i = 0
for ; i < 2; i = i + 1; print i
// [Expect] 0
// [Expect] 1

// No condition.
fn bar() {
  for let i = 0;; i = i + 1; {
    print i
    if i >= 2 ret
  }
}
bar()
// [Expect] 0
// [Expect] 1
// [Expect] 2

// No increment.
for let i = 0; i < 2;; {
  print i
  i = i + 1
}
// [Expect] 0
// [Expect] 1

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
print stat_body_1() // [Expect] 1
print stat_body_2() // [Expect] 1
print stat_body_3() // [Expect] 1
