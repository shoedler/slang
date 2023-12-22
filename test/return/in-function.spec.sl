fn f() {
  ret "ok";
  print "bad"
}

print f() // [Expect] ok
