# Dev

I started this project on a Windows 11 Machine with Visual Studio 2022 Community Edition and d C/C++ Workload.
Since I'm new to GCC, and likely forget the process in the future, I'll document the steps on installing GCC on Windows 11.

## Toolchain Setup for Windows 11

> [!NOTE]
>
> We'll be using MSYS2' UCRT64 toolchain exclusively for this project. It's more modern, linking against the Universal C Runtime
> (UCRT) instead of the older MSVCRT. This is a personal preference and might not be suitable for all projects.

1. Install [MSYS2](https://www.msys2.org/) (I installed it in `C:\Projects\.dev\msys64`)
2. Optionally, add a windows Terminal profile for the UCRT64 shell:

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
- _clang-tidy_ with `$ pacman -S mingw-w64-ucrt-x86_64-clang-tools-extra`
- _make_ with `$ pacman -S make`
- ~~_jemalloc_ with `$ pacman -S mingw-w64-ucrt-x86_64-jemalloc` (Since we require a concurrent memory allocator)~~
- _mimalloc_ with `pacman -S mingw-w64-ucrt-x86_64-mimalloc`

> [!NOTE]
>
> Also, if you want to rebuild the _compile_commands.json_ file - required for `clang-tidy`, you can install `compiledb` with `$ pip install compiledb`.
> (Requires Python and pip to be installed on your host machine)
> This is necessary if you change the build system or add new files to the project. Run it with `compiledb make release` in the project root.

5. Add relevant bin dirs to the PATH environment variable of your host machine:

- `C:\Projects\.dev\msys64\ucrt64\bin` (for gcc and gdb as well as jemalloc)
- `C:\Projects\.dev\msys64\usr\bin` (for make)

Now, you should be able to build the project with `make`.

## Enhancing Performance with Profile-Guided Optimization (PGO)

> [!NOTE]
>
> There's already a target in the Makefile for PGO. Just run `make release-profiled` to build with PGO. This will utilize the `profile.sl` file in the project root to optimize the build.
> This short section is intended to provide a brief overview of PGO.

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

## Building a jemalloc Debug-build with UCRT64 for Windows

> [!WARNING]
>
> This section is a quick documentation of the steps I took to build jemalloc with UCRT64 on Windows 11. It's not a complete guide and might not work for you.
> I recommend reading the official documentation and adapting the steps to your environment. However, it might give you a starting point.

All the commands are executed in the UCRT64 shell.

1. Install tools

```sh
pacman -R jemalloc # r  Remove current installation - if any
pacman -S base-devel # ensure you have build tools
pacman -S autoconf # needed for jemalloc's configure script
```

2. Get jemalloc source (In Nov 2024, 5.0.3 was the latest version)

```sh
wget https://github.com/jemalloc/jemalloc/archive/refs/tags/5.3.0.tar.gz
tar xzf 5.3.0.tar.gz
```

3. Run autoconf _(I think this is only required initially after fetching the source.)_

```sh
$build_env autoconf
```

4. Configure build environment. As per [INSTALL.md](https://github.com/jemalloc/jemalloc/blob/dev/INSTALL.md), we need to run `before_install.sh` "manually" (located in _~/jemalloc-5.3.0/scrips/windows_). I used the version from 5.3.0 and modified it to use the UCRT64 tools - basically removing some of the checks and renaming `mingw64` to `ucrt64`. Resulting in the following script:

```sh
#!/bin/bash

set -e

msys_shell_cmd="cmd //C RefreshEnv.cmd && set MSYS=winsymlinks:nativestrict && C:\\Projects\\.dev\\msys64\\msys2_shell.cmd"

ucrt64() { $msys_shell_cmd -defterm -no-start -ucrt64 -c "$*"; }

mingw=ucrt64
mingw_gcc_package_arch=x86_64

$mingw pacman -S --noconfirm --needed \
    autotools \
    git \
    mingw-w64-ucrt-${mingw_gcc_package_arch}-make \
    mingw-w64-ucrt-${mingw_gcc_package_arch}-gcc \
    mingw-w64-ucrt-${mingw_gcc_package_arch}-binutils
build_env=$mingw

echo "Build environment function: $build_env"
```

Make a new file `before_build_ucrt64.sh` in _~/jemalloc-5.3.0/scrips/windows_ and paste the script above into it. Then run the script in the UCRT64 shell.
You'll likely get an error about `RefreshEnv.cmd` not being found.
Add the `RefreshEnv.cmd` file somewhere (I use a _.dev/$PATH_ folder on my machine for stuff like this). I used this script: https://stackoverflow.com/a/32420542
Then, add the path to the UCRT64 shell profile:

```sh
PATH="$PATH:/c/Projects/.dev/\$PATH"
```

Then run the script again. It should install the necessary tools and set up the build environment. Use `$build_env` to run the commands in the UCRT64 shell.

5. Configure jemalloc

```sh
$build_env ./configure \
 --prefix=/ucrt64 \
 --with-version=5.3.0-0-g54eaed1d8b56b1aa528be3bdd1877e59c56fa90c \
 --with-malloc-conf=background_thread:true,metadata_thp:disabled,dirty_decay_ms:30000,muzzy_decay_ms:30000,abort:true,abort_conf:true,prof:true,prof_active:true,confirm_conf:true \
 --enable-debug \
 --enable-prof \
 --disable-cxx \
 --with-jemalloc-prefix=je\_
```

6. Build jemalloc and install it

```sh
$build_env make
$build_env make install
```

Now, theoretically, you should be able to just build your project with jemalloc. I had this in my Makefile:

```makefile
# Get jemalloc configuration
JEMALLOC_LIBS := $(shell jemalloc-config --libs)
JEMALLOC_LIBDIR := $(shell jemalloc-config --libdir)
JEMALLOC_LDFLAGS := -L$(JEMALLOC_LIBDIR) -Wl,-rpath,$(JEMALLOC_LIBDIR) -ljemalloc

# Add to existing flags
LIBS += $(JEMALLOC_LIBS) $(JEMALLOC_LDFLAGS)
```

This should automatically link the self-built jemalloc library to your project.

That should be it. After that, these commands might be helpful for further development:

- Before installing a new build, you should uninstall the previous one:

```sh
$build_env make uninstall
```

- To help find the binaries in your UCRT64 directory, you can use the following command:

```sh
find /ucrt64 -name "_jemalloc_"
```
