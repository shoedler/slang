print !true     // [Expect] false
print !false    // [Expect] true
print !!true    // [Expect] true

print !123      // [Expect] false
print !0        // [Expect] false

print !nil     // [Expect] true

print !""       // [Expect] false

fn foo() {}
print !foo      // [Expect] false
