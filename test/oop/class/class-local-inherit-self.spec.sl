// [exit] 2
{
  cls Foo : Foo {} // [expect-error] Parser error at line 3 at 'Foo': A class can't inherit from itself.
}                  // [expect-error]      3 |   cls Foo : Foo {}
                   // [expect-error]                      ~~~