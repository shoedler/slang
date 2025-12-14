import Gc
import Perf

Gc.stress(true) // This is set to true by default in the test runner - just to be explicit

fn test_garbage_generation {
  const prev_freed = Gc.collect()
  
  // Generate garbage by creating lots of temporary objects
  for let i = 0; i < 100000; i++; {
    const temp = {
      "array": [1, 2, 3, 4, 5],
      "string": "This is a long string that will be garbage",
      "nested": {
        "more": [1, 2, 3],
        "data": "More garbage data"
      }
    }
  }

  const freed = Gc.collect()
  print freed - prev_freed        
  print Gc.stats().bytes_allocated
  Gc.collect()
}

Gc.collect()
test_garbage_generation()// [expect] 944
                         // [expect] 49181
test_garbage_generation()// [expect] 944
                         // [expect] 49181
test_garbage_generation()// [expect] 944
                         // [expect] 49181

print Gc.collect() // [expect] 0
