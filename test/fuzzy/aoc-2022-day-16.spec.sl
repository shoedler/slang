throw "Currently does not return the right results. Parsing is correct, but something is wrong with the logic."
// [ExpectRuntimeError] Uncaught error: Currently does not return the right results. Parsing is correct, but something is wrong with the logic.
// [ExpectRuntimeError] at line 1 at the toplevel of module "main"

import File
import Perf

let flow_rates = []
let tunnels = []

let valve_labels = {}
let num_valves = 0

File
  .read(cwd() + "aoc-2022-day-16.txt")
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
  if !(vid is Int) throw "vid must be an integer, not " + typeof(vid).__name
  if !(opened is Tuple) throw "opened must be tuple, not " + typeof(opened).__name
  if !(min_left is Int) throw "min_left must be an integer, not " + typeof(min_left).__name
  if !(type is Int) throw "type must be an integer, not " + typeof(type).__name

  if min_left <= 0 {
    ret type == 1 ? 0 : max_relief(valve_labels["AA"], opened, 26, 1); // It's the elephants turn now!
  }

  // Check if we already have a result for this signature
  let cache_key = (vid, min_left, opened)
  if cache[type][cache_key] is Int {
    ret cache[type][cache_key];
  }

  let max_reliefed = 0

  // Unopened
  if opened[vid] == false and flow_rates[vid] > 0 {
    let valve_relief = (min_left - 1) * flow_rates[vid]

    let prev_opened = Seq(opened)
    prev_opened[vid] = true
    let new_opened = Tuple(prev_opened)

    tunnels[vid].each(fn (next_valve) {
      let next_vid = valve_labels[next_valve]
      let reliefed = valve_relief + max_relief(next_vid, new_opened, min_left - 2, type)
      max_reliefed = reliefed > max_reliefed ? reliefed : max_reliefed
    })
  }
 
  // Opened
  tunnels[vid].each(fn (next_valve) {
    let next_vid = valve_labels[next_valve]
    let reliefed = max_relief(next_vid, opened, min_left - 1, type)
    max_reliefed = reliefed > max_reliefed ? reliefed : max_reliefed
  })

  cache[type][cache_key] = max_reliefed
  ret max_reliefed;
}

let opened = Tuple(Seq(num_valves + 1).map(fn (x) -> false))

let start = Perf.now()
let result = max_relief(valve_labels["AA"], opened, 30, 1)
let time = Perf.now() - start

// print "Part 1: " + result.to_str() + " took " + time.to_str() + "s"
print result // /Expect/ 1460

start = Perf.now()
result = max_relief(valve_labels["AA"], opened, 26, 2)
time = Perf.now() - start

// print "Part 2: " + result.to_str() + " took " + time.to_str() + "s"
print result // /Expect/ 2117

