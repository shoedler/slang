// [exit] 2
import { a, b } from "/i/dont/know"
a = nil // [expect-error] Compile error at line 3 at 'a': Can't reassign a constant.
