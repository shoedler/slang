# Legacy Benchmark Results

## Fib35

| CHANGE                     | COMMIT HASH                              | FIB # | t DEBUG | t RELEASE |
| -------------------------- | ---------------------------------------- | ----- | ------- | --------- |
| After adding classes       | 03374177fea58b99ab4a1c47014aca8855a50485 | 35    | 6.732s  | 0.837s    |
| After book + optimizations | 6e4ac55250f254043d6049bfaef5476ca71abea0 | 35    | 6.589s  | 0.819s    |

# 2023-12-28T23:45:48.862Z

- Commit Hash: `3e5b3443be202bdf71a17a137df7a0953081cbec`
- Commit Date: `2023-12-27 23:33:07 +0100`
- Commit Message: `Adds code doc VIII`
- Processor: `13th Gen Intel(R) Core(TM) i7-13700H`

| Benchmark | Config | Result |
| --- | --- | --- |
| fib | Release | `Fib of 35: 9227465 Took (s): 0.833000 ` |
| zoo | Release | `Batches of 10k: 3457 Sum: 207420000 Duration: 5 ` |
| fib | Debug | `Fib of 35: 9227465 Took (s): 7.222000 ` |
| zoo | Debug | `Batches of 10k: 365 Sum: 21900000 Duration: 5 ` |

# 2024-01-01T10:09:12.469Z

- ~~Commit Hash: `ce31ec9df8dd31476c7bd2c01be03077e38ac57c`~~
- ~~Commit Date: `2023-12-31 14:48:48 +0100`~~
- ~~Commit Message: `Adds sequences 🎉 (Up to UINT8_MAX in this commit)`~~
- Processor: `13th Gen Intel(R) Core(TM) i7-13700H`

⚠️ This is with a 32-bit stack width, vs 8-bit for the previous benchmarks.

| Benchmark | Config | Result |
| --- | --- | --- |
| fib | Release | `Fib of 35: 9227465 Took (s): 0.875000 ` |
| zoo | Release | `Batches of 10k: 3379 Sum: 202740000 Duration: 5 ` |
| fib | Debug | `Fib of 35: 9227465 Took (s): 7.252000 ` |
| zoo | Debug | `Batches of 10k: 365 Sum: 21900000 Duration: 5 ` |

# 2024-01-01T10:47:08.047Z

- ~~Commit Hash: `ce31ec9df8dd31476c7bd2c01be03077e38ac57c`~~
- ~~Commit Date: `2023-12-31 14:48:48 +0100`~~
- ~~Commit Message: `Adds sequences 🎉 (Up to UINT8_MAX in this commit)`~~
- Processor: `13th Gen Intel(R) Core(TM) i7-13700H`

⚠️ This is with a 16-bit stack width, vs 8-bit for the previous benchmarks.

| Benchmark | Config | Result |
| --- | --- | --- |
| fib | Release | `Fib of 35: 9227465 Took (s): 0.825000 ` |
| zoo | Release | `Batches of 10k: 3458 Sum: 207480000 Duration: 5 ` |
| fib | Debug | `Fib of 35: 9227465 Took (s): 7.313000 ` |
| zoo | Debug | `Batches of 10k: 377 Sum: 22620000 Duration: 5 ` |

