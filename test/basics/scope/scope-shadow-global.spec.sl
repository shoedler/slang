let a = "global"
{
  let a = "shadow"
  print a // [expect] shadow
}
print a // [expect] global
