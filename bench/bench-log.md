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

