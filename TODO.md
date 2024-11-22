
- Replace [ExpectError] and [Expect] and [Exit] with lower-case versions
  - Then, find ([a-z])([A-Z]) and replace with $1\_$2, though $2 will still be upper-case
- Maybe check if we can replace other #define Macros with flags to toggle

