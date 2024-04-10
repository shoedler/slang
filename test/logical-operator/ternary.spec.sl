print {} ? "truthy" : "(skip)" // [Expect] truthy

print nil ? 
        "(skip)" : 
        false ? 
          "(skip)" : 
          true ? 
            "truthy" : 
            "(skip)"           // [Expect] truthy

let is_nice = true
let a = (is_nice ? fn -> "nice" : fn -> "not nice")()
print a // [Expect] nice