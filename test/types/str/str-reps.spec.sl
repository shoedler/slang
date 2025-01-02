print "".reps(0) + "." // [expect] .
print "".reps(1) + "." // [expect] .
print "".reps(1000) + "." // [expect] .

print "a".reps(0) + "." // [expect] .
print "a".reps(1) + "." // [expect] a.
print "a".reps(10) + "." // [expect] aaaaaaaaaa.

print "abc".reps(0) + "." // [expect] .
print "abc".reps(1) // [expect] abc
print "abc".reps(2) // [expect] abcabc
print "abc".reps(3) // [expect] abcabcabc

print "abc".reps(-1) + "." // [expect] .
