// List of all the native modules embedded in the runtime:
const native = ["Debug", "File", "Perf", "Gc", "Math"]

import Debug

const module_cache = Debug.modules()

module_cache.keys().each(fn (module) {
  if !(module in native) {
    print "Module '" + module + "' is also in the module cache"
  }
})