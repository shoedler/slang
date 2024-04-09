import File
// import Perf

fn is_digit(x) ->
  x == "0" or x == "1" or x == "2" or x == "3" or x == "4" or
  x == "5" or x == "6" or x == "7" or x == "8" or x == "9"

fn get_calibration_values_of_line(line) {
  let arr = line.split("")
  ret Num(arr.first(is_digit) + arr.last(is_digit));
}

// let start = Perf.now()

print File
  .read(cwd() + "aoc-2023-day-1-part-1.txt")
  .split("\r\n")
  .map(get_calibration_values_of_line)
  .reduce(0, fn(acc, x) -> acc + x) // [Expect] 54951

// print ((Perf.now() - start ) * 1000).to_str() + "ms"