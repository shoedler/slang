import File
import { Set } from "../../modules/std.sl"

const num_word_map = {"one": "1", "two": "2", "three": "3", "four": "4", "five": "5", "six": "6", "seven": "7", "eight": "8", "nine": "9",}
const num_word_lens = Set(num_word_map.keys().map(fn (x) -> x.len)).values()

const digits = "0123456789"

fn is_digit(x) -> x in digits

fn to_calibration_value(line) {
  const arr = line.split("")
  ret Int(arr.first(is_digit) + arr.last(is_digit))
}

fn to_calibration_value_2(line) {
  let new = ""

  for let i = 0; i < line.len; i++; {
    if is_digit(line[i]) { 
      new += line[i]
    }
    else {
      num_word_lens.some(fn (j) {
        const snip = line[i..i+j]
        const num = num_word_map[snip]
        if num {
          new += num
          ret true
        }
      })
    }
  }

  ret to_calibration_value(new)
}

// Part 1
print File
  .read(cwd() + "fuzzy-aoc-2023-day-01.txt")
  .split("\r\n")
  .map(to_calibration_value)
  .fold(0, fn(acc, x) -> acc + x) // [expect] 54951

// Part 2
print File
  .read(cwd() + "fuzzy-aoc-2023-day-01.txt")
  .split("\r\n")
  .map(to_calibration_value_2)
  .fold(0, fn(acc, x) -> acc + x) // [expect] 55218 

