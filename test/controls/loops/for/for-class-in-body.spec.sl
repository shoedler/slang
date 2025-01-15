// [exit] 2
for ;;; cls Foo {} // [expect-error] Parser error at line 2 at 'cls': Expecting expression.
                   // [expect-error]      2 | for ;;; cls Foo {}
                   // [expect-error]                  ~~~