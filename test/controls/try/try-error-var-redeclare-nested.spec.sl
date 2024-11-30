const error = 321
{
  const error = 123
  {
    const error = "foo"
    print error // [expect] foo
  }
  print error // [expect] 123
}
print error // [expect] 321