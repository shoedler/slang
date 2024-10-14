# Dev Notes

## Toolchain

I started this project on a Windows 11 Machine with Visual Studio 2022 Community Edition and d C/C++ Workload.
I moved to GCC as a starting point for cross-platform compatibility.
Since I'm new to GCC, and likely forget the process in the future, I'll document the steps on installing GCC on Windows 11.

1. Install [MSYS2](https://www.msys2.org/) (I installed it in `C:\Projects\.dev\msys64`. This is where the vscode profiles expect it to be).
2. Optionally, add a windows Terminal profile for the UCRT64 (Universal C Runtime) shell:

```json
{
  "closeOnExit": "always",
  "commandline": "C:/Projects/.dev/msys64/msys2_shell.cmd -defterm -here -no-start -ucrt64 -shell bash",
  "guid": "{98dc0aba-2281-4e74-9a81-e9d3b28c2828}",
  "hidden": false,
  "icon": "%SystemDrive%/Projects/.dev/msys64/ucrt64.ico",
  "name": "UCRT64",
  "startingDirectory": "C:\\Projects"
}
```

3. Open a terminal either through the new profile or ucrt64.exe in the MSYS2 folder.
4. Install packages:

- _gcc_ (UCRT64) with the more modern UCRT with `$ pacman -S mingw-w64-ucrt-x86_64-gcc` in the MSYS2 terminal.
- _gdb_ (UCRT64) with `$ pacman -S ucrt64/mingw-w64-ucrt-x86_64-gdb`
- _make_ with `$ pacman -S make`

4. Add relevant bin dirs to the PATH environment variable:

- `C:\Projects\.dev\msys64\ucrt64\bin` (for gcc and gdb)
- `C:\Projects\.dev\msys64\usr\bin` (for make)

Now, you should be able to build the project with `make`.

## Enhancing Performance with Profile-Guided Optimization (PGO)

PGO involves running your program with typical input to collect profiling data, which is then used to optimize the final build. This can significantly improve performance because the compiler has real usage data to optimize around.

```makefile
# Add these steps to your Makefile to use PGO:
RELEASE_CFLAGS += -fprofile-generate
RELEASE_LDFLAGS += -fprofile-generate

# After running the program to generate profile data:
RELEASE_CFLAGS := $(filter-out -fprofile-generate, $(RELEASE_CFLAGS)) -fprofile-use
RELEASE_LDFLAGS := $(filter-out -fprofile-generate, $(RELEASE_LDFLAGS)) -fprofile-use -lgcov
```

You'll need to build and run the executable once to collect the profile data, then rebuild it with `-fprofile-use` to optimize using that data.

> [!NOTE]
> PGO is available trough the `release-profiled` target in the Makefile (Also implicitly used in `all`). The normal `release` target does not use PGO.
> The profiled build will use _profile.sl_ as the slang input file.
