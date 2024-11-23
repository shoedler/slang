print {} ? "truthy" : "(skip)" // [expect] truthy

print nil ? 
        "(skip)" : 
        false ? 
          "(skip)" : 
          true ? 
            "truthy" : 
            "(skip)"           // [expect] truthy

let is_nice = true
let a = (is_nice ? fn -> "nice" : fn -> "not nice")()
print a // [expect] nice