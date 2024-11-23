fn f() {
  ret "ok"
  print "bad"
}

print f() // [expect] ok
