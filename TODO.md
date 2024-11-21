- Remove DEBUG_STRESS_GC in favor of the new VM flag
- Add cli option to enable it and use it in test runs
- Fix prealloc_value_array -> it should not be looped over, that's missing the point of it!
  Check if we can initialize the count with Zero and then increment as we go.
- Replace [ExpectError] and [Expect] and [Exit] with lower-case versions
  - Then, find ([a-z])([A-Z]) and replace with $1\_$2, though $2 will still be upper-case
- Maybe check if we can replace other #define Macros with flags to toggle
- Find a better name for Gc.toggle_force, maybe Gc.stress?
