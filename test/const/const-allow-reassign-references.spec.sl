// Sanity check after implementing const: Reassigning a class or a named function should be allowed
cls P {}
fn foo -> 1

print P // [Expect] <Class P>
print foo // [Expect] <Fn foo>

P = 1
foo = 2

print P // [Expect] 1
print foo // [Expect] 2
