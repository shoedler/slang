// [exit] 2
import () // [expect-error] Parser error at line 2 at '(': Expecting identifier or '{' after 'import'.
          // [expect-error]      2 | import ()
          // [expect-error]                 ~
          // [expect-error] Parser error at line 2 at ')': Expecting expression.
          // [expect-error]      2 | import ()
          // [expect-error]                  ~
          // [expect-error] Parser error at line 2 at end: Expecting ')' after grouping expression.
          // [expect-error]      2 | import ()
          // [expect-error] 