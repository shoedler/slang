
import Gc
import Perf

Gc.toggle_force(true)

fn test_keep_alive {  
  // Create a class that will hold circular references
  cls CircularRef {
    ctor {
      this.value = "original"
      this.others = []
      this.closures = []
      this.self_ref = this  // Circular reference to self
    }
    
    fn add_circular(other) {
      this.others.push(other)  // Create circular reference chain
      other.others.push(this)
    }
    
    fn create_closure {
      // Create a closure that captures 'this'
      let captured = this
      let closure = fn {
        ret captured.value
      }
      this.closures.push(closure)
      ret closure
    }
    
    fn modify_value(new_val) {
      this.value = new_val
    }
  }
  
  // Create a complex web of objects and references
  let refs = []
  let closures = []
  
  // Create several objects that reference each other
  for let i = 0; i < 5; i++; {
    refs.push(CircularRef())
  }
  
  // Create circular reference chains between all objects
  for let i = 0; i < refs.len; i++; {
    for let j = i + 1; j < refs.len; j++; {
      refs[i].add_circular(refs[j])
    }
  }
  
  // Create nested closures that capture various objects
  fn create_nested_closure(obj, depth) {
    if depth == 0 ret fn { ret obj.value }
    
    let next_closure = create_nested_closure(obj, depth - 1)
    ret fn {
      let val = next_closure()
      ret obj.value + val
    }
  }
  
  // Create deeply nested closures for each object
  refs.each(fn (ref) {
    closures.push(create_nested_closure(ref, 10))  // 10 levels deep
  })
  
  // Create cross-referencing closures
  refs.each(fn (ref, i) {
    let next_ref = refs[(i + 1) % refs.len]
    let cross_closure = fn {
      ref.modify_value(next_ref.value)
      ret ref.value
    }
    closures.push(cross_closure)
    ref.closures.push(cross_closure)
  })
  
  // Create an object that holds weak references to test proper cleanup
  let weak_refs = {
    "temp1": [1, 2, 3],
    "temp2": { "a": 1, "b": 2 }
  }
  
  // Force garbage collection
  let before_stats = Gc.stats()
  Gc.collect()
  let after_stats = Gc.stats()
  

  // Test that all objects are still accessible
  refs.each(fn (ref, i) {
    if ref.self_ref != ref throw "Self reference broken"
    if ref.others.len != refs.len - 1 throw "Cross references broken"
  })
  
  // Test all closures still work
  closures.each(fn (closure) {
    let result = closure()
    if result == nil throw "Closure failed"
  })
  
  // Modify values through closures and verify changes propagate
  let test_value = "modified_after_gc"
  refs[0].modify_value(test_value)
  
  // Verify the change propagated through the circular references
  let propagation_closure = closures[closures.len - 1]  // Get last cross-reference closure
  let result = propagation_closure()
  if result != test_value throw "Propagation failed"
  
  print "All references and closures survived collection properly"
  // print "Memory before test: " + before_stats.bytes_allocated.to_str()
  // print "Memory after test:  " + after_stats.bytes_allocated.to_str()
}

test_keep_alive() // [Expect] All references and closures survived collection properly

