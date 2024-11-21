import Gc
import Perf

fn test_garbage_generation {
  const initial_allocd = Gc.stats().bytes_allocated
  
  // Generate garbage by creating lots of temporary objects
  for let i = 0; i < 10000; i++; {
    const temp = {
      "array": [1, 2, 3, 4, 5],
      "string": "This is a long string that will be garbage",
      "nested": {
        "more": [1, 2, 3],
        "data": "More garbage data"
      }
    }
  }
  
  const before_forced_allocd = Gc.stats().bytes_allocated
  const freed = Gc.collect()
  const after_forced_allocd = Gc.stats().bytes_allocated

  print initial_allocd       // [Expect] 25519
  print before_forced_allocd // [Expect] 30239
  print freed                // [Expect] 5126
  print after_forced_allocd  // [Expect] 25519
}

test_garbage_generation()

print Gc.collect() // [Expect] 543
