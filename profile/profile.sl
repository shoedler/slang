import File
import Perf

let flow_rates = []
let tunnels = []

let valve_labels = {}
let num_valves = 0

File
  .read(cwd() + "input.txt")
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

let opened = Tuple(Seq(num_valves + 1).map(fn -> false))

let start = Perf.now()
let result = max_relief(valve_labels["AA"], opened, 30, 1)
let time = Perf.now() - start

print "Result: " + result + " took " + time + "s" + ", cache size " + cache[1].len
// 1460 took 1.05499110004166s
