This is a C based repository with a Deno.js runner for testing and benchmarking and other utilities for development. The runner is not required for using the language.
It is a compiler and runtime for the Slang programming language. Please follow these guidelines when contributing:

## Code Standards

### Development Flow

- Build: `make debug` or `make release`
- Test: `cd ./runner && deno task test no-build`. The `no-build` flag skips the build step and runs the tests directly. The runner utilizes the latest `release` build for the test target.

## Repository Structure

- `.vscode/`: Visual Studio Code configuration files
- `bench/`: Benchmark implementations in Slang, Js and Python
- `doc/`: Documentation files
- `modules/`: Slang modules
- `profile/`: Slang Programs for PGO-builds
- `runner/`: Deno.js runner for testing and benchmarking
- `slang-syntax/`: TextMate syntax definitions for Slang (vscode extension)
- `./`: Root directory for the Slang project - all source code and assets live here

## Key Guidelines

1. Follow C best practices and idiomatic patterns
2. Maintain existing code structure and organization
3. Write tests for new functionality. Take inspiration from existing tests and follow the same patterns.
4. Document public APIs and complex logic. Suggest changes to the `docs/` folder when appropriate
