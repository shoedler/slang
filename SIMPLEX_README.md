# Simplex Algorithm in Slang

This is a conversion of a Python program implementing the Simplex Algorithm and Integer Linear Programming solver to Slang.

## Features

- **Simplex Algorithm**: Implementation of the simplex method for linear programming
- **Branch-and-Bound ILP**: Integer Linear Programming solver using branch and bound
- **Part 1**: BFS solution using bitmasks
- **Part 2**: ILP-based solution

## Usage

Create an `input.txt` file in the same directory with the following format:

```
<mask> <tuple1> <tuple2> ... <costs>
```

Example:
```
##..## (0,1) (2,3) [1,2,3,4,5,6]
```

Where:
- `<mask>` is a string of `#` and `.` characters (# = 1, . = 0)
- `<tuples>` are space-separated coordinate pairs like `(0,1)`
- `<costs>` is an array of integers like `[1,2,3,4,5,6]`

Run the program:
```bash
slang run simplex.sl
```

## Conversion Notes

The original Python code was converted to Slang with the following changes:

1. **Float infinity**: Use `Float.inf` instead of `float('inf')`
2. **Scientific notation**: Replace `1e-9` with `0.000000001`
3. **Bitwise operations**: Use `Math.shl`, `Math.shr`, `Math.xor` instead of operators `<<`, `>>`, `^`
4. **String methods**: Implement character replacement manually instead of `str.replace()`
5. **File I/O**: Use `File.read(File.join_path(cwd(), "input.txt"))` instead of `open(0)`
6. **List comprehensions**: Expand to explicit loops
7. **Lambda functions**: Use Slang's anonymous function syntax `fn(...) -> ...`
8. **Generator expressions**: Expand to manual loops with early termination
9. **Walrus operator**: Split into separate assignment and usage
10. **eval()**: Parse tuples manually from string format

## Algorithm Overview

### Simplex Method
The simplex algorithm solves linear programming problems by:
1. Converting to standard form with slack variables
2. Finding an initial basic feasible solution
3. Iteratively pivoting to improve the objective function
4. Terminating when optimal or unbounded

### Branch-and-Bound ILP
The integer linear programming solver uses:
1. Relaxation to solve LP at each node
2. Branching on fractional variables
3. Bounding to prune suboptimal branches
4. Depth-first search strategy

### Part 1: Bitmask BFS
- Uses bitwise operations to represent state
- BFS to find shortest path to target state
- Each button press flips specific bits

### Part 2: ILP Formulation
- Formulates constraints as linear inequalities
- Uses ILP solver to find integer solution
- Minimizes cost based on button press counts
