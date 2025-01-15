let a = "outer"
{           // [exit] 2
  let a = a // [expect-error] Resolver error at line 3: Cannot read variable in its own initializer.
}           // [expect-error]      3 |   let a = a
            // [expect-error]                    ~