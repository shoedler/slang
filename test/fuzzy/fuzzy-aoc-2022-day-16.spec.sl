// [skip] Too slow

// Note: This program was used to implement the parallel Gc.

import File
import Perf
import Gc

let flow_rates = []
let tunnels = []

let valve_labels = {}
let num_valves = 0

fn solve {
  File
    .read(cwd() + "fuzzy-aoc-2022-day-16.txt")
    .split("\r\n")
    .each(fn (line, i) {
      let [left, right] = line.split(";")

      // Left side, "Valve A has flow rate=10"
      let [valve_label, flow_rate] = left.split(" has flow rate=")
      valve_label = valve_label.slice("Valve ".len, nil)
      flow_rate = Int(flow_rate)

      // Right side, " tunnels lead to valves B, C, D"
      let connections = right.slice(" tunnels lead to valves ".len, nil)
      connections = connections.split(", ")

      flow_rates.push(flow_rate)
      tunnels.push(connections)
      valve_labels[valve_label] = i
      num_valves = i
    })

  let cache = {
    1: {},
    2: {},
  }

  fn max_relief(vid, opened, min_left, type) {
    // Base case
    if min_left <= 0 {
      ret type == 1 ? 0 : max_relief(valve_labels["AA"], opened, 26, 1) // It's the elephants turn now!
    }

    // Check if we already have a result for this signature
    let cache_key = (vid, min_left, opened)
    if cache[type][cache_key] is Int {
      ret cache[type][cache_key]
    }

    let max_reliefed = 0

    let tunnel = tunnels[vid]

    // If the valve is closed, we can only open it. With a simple heuristic: Does it have a flow rate?
    // Bc if it doesn't, it cannot add any relief. It's more of a plug than a valve - lol.
    if opened[vid] == false and flow_rates[vid] > 0 {
      let valve_relief = (min_left - 1) * flow_rates[vid]

      // Open the valve by converting the Tuple to a Seq, and then back to a Tuple
      let prev_opened = Seq(opened)
      prev_opened[vid] = true
      let new_opened = Tuple(prev_opened)

      for let i = 0; i < tunnel.len; i++ ; {
        // We subtract 2 from min_left because it takes 1 minute to open the valve and 1 minute to walk trough the tunnel.
        let reliefed = valve_relief + max_relief(valve_labels[tunnel[i]], new_opened, min_left - 2, type)
        max_reliefed = reliefed > max_reliefed ? reliefed : max_reliefed
      }
    }
  
    // Also run the case where we don't open the valve
    for let i = 0; i < tunnel.len; i++ ; {
      // In this case, we only subtract 1 from min_left, because we only need to walk trough the tunnel to the next valve.
      let reliefed = max_relief(valve_labels[tunnel[i]], opened, min_left - 1, type)
      max_reliefed = reliefed > max_reliefed ? reliefed : max_reliefed
    }

    // Now store that baby in the cache using tuple hashing
    cache[type][cache_key] = max_reliefed
    ret max_reliefed
  }

  let opened = Tuple(Seq(num_valves + 1).map(fn (x) -> false))

  let start = Perf.now()
  let result = max_relief(valve_labels["AA"], opened, 30, 1)
  let time = Perf.now() - start

  print "Part 1: " + result.to_str() + " took " + time.to_str() + "s"
  print "Cache size " + cache[1].len.to_str()
  
  start = Perf.now()
  result = max_relief(valve_labels["AA"], opened, 26, 2)
  time = Perf.now() - start

  print "Part 2: " + result.to_str() + " took " + time.to_str() + "s"
  print "Cache size " + cache[2].len.to_str()
}

solve() // In a fn, so we can collect the garbage after it's done to test the garbage collector

print "-------------------"
print "Done! - Collecting final garbage"

Gc.collect()

print "-------------------"

// Slang output:
// Part 1: 1460 took 1.19803839999986s
// Cache size 573172
// Part 2: 2117 took 148.8982759999999s (2m 28s)
// Cache size 244166

// Slang output (Without the solve-fn wrapper):
// Part 1: 1460 took 0.89002869999968s
// Cache size 573172
// Part 2: 2117 took 139.40825869999935s (2m 19s)
// Cache size 244166

// Js output (Ran on a tiny bit slower machine):
// Part One: 1460 took 1.139946400s
// Cache size 573172
// Part Two: 2117 took 49.342724400s
// Cache size 244166
