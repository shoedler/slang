let a = "global"
{
  let a = "shadow"
  print a // [Expect] shadow
}
print a // [Expect] global
