// Sanity check after implementing const: Reassigning a class or a named function should be allowed
cls P {}
fn foo -> 1

print P // [expect] <P>
print foo // [expect] <Fn foo>

P = 1
foo = 2

print P // [expect] 1
print foo // [expect] 2
