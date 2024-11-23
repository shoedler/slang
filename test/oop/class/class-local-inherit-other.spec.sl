cls A {}

fn f {
  cls B : A {}
  ret B
}

print f() // [expect] <Class B>
