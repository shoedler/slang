import File

let nums = {"one": "1", "two": "2", "three": "3", "four": "4", "five": "5", "six": "6", "seven": "7", "eight": "8", "nine": "9",}
let num_lens = [3,4,5]

fn is_digit(x) -> 
  x == "0" or x == "1" or x == "2" or x == "3" or x == "4" or
  x == "5" or x == "6" or x == "7" or x == "8" or x == "9"

fn to_calibration_value(line) {
  let arr = line.split("")
  ret Num(arr.first(is_digit) + arr.last(is_digit));
}

fn substr(s, start, end) {
  let res = ""
  for let i = start; i < end and i < s.len(); i++; {
    res = res + s[i]
  }
  ret res;
}

fn to_calibration_value_2(line) {
  let arr = line.split("")
  let new = []

  for let i = 0; i < arr.len(); i++; {
    if is_digit(arr[i]) { 
      new.push(arr[i])
    }
    else {
      num_lens.some(fn (j) {
        let slice = substr(arr, i, i + j)
        let num = nums[slice]
        if num {
          new.push(num)
          ret true;
        }
      })
    }
  }

  ret to_calibration_value(new.join(""));
}

// Part 1
print File
  .read(cwd() + "aoc-2023-day-1.txt")
  .split("\r\n")
  .map(to_calibration_value)
  .reduce(0, fn(acc, x) -> acc + x) // [Expect] 54951

// Part 2
print File
  .read(cwd() + "aoc-2023-day-1.txt")
  .split("\r\n")
  .map(to_calibration_value_2)
  .reduce(0, fn(acc, x) -> acc + x) // [Expect] 55218 

