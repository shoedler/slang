CC=gcc
LD=gcc
RM=rm -f
MKDIR=mkdir -p

# Hide make commands
MAKEFLAGS += --no-print-directory 

# General compiler flags
CFLAGS=-Wall -Wextra -Werror -std=c17 -m64 -D_UNICODE -DUNICODE
LDFLAGS=-m64
LIBS=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -lmimalloc

# Source files and output
DEBUG_EXEC=bin/x64/debug/slang.exe
RELEASE_EXEC=bin/x64/release/slang.exe
DEBUG_DIR=bin/x64/debug/
RELEASE_DIR=bin/x64/release/
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
DEBUG_OBJECTS=$(addprefix $(DEBUG_DIR), $(OBJECTS))
RELEASE_OBJECTS=$(addprefix $(RELEASE_DIR), $(OBJECTS))
DEBUG_DEP_DIR=$(DEBUG_DIR).deps/
RELEASE_DEP_DIR=$(RELEASE_DIR).deps/

# Dependency generation flags
DEBUG_DEP_CFLAGS=-MMD -MP -MF $(DEBUG_DEP_DIR)/$*.d
RELEASE_DEP_CFLAGS=-MMD -MP -MF $(RELEASE_DEP_DIR)/$*.d

# Mimalloc library path
MIMALLOC_LIBDIR := /ucrt64/lib
LDFLAGS += -L$(MIMALLOC_LIBDIR) -Wl,-rpath,$(MIMALLOC_LIBDIR)

# Debug specific flags
DEBUG_CFLAGS=$(CFLAGS) -g -O0 -D_DEBUG
DEBUG_LDFLAGS=$(LDFLAGS) -g 

# Number of parallel LTO threads
LTO_THREADS=20

# Release specific flags (always include LTO for release builds)
RELEASE_CFLAGS=$(CFLAGS) -O3 -march=native -DNDEBUG -flto=$(LTO_THREADS)
RELEASE_LDFLAGS=$(LDFLAGS) -flto=$(LTO_THREADS)

# Profile generation flags
PROFILING_CFLAGS=$(RELEASE_CFLAGS) -fprofile-generate
PROFILING_LDFLAGS=$(RELEASE_LDFLAGS) -fprofile-generate

# Final profiled build flags
FINAL_CFLAGS=$(RELEASE_CFLAGS) -fprofile-use -fprofile-correction
FINAL_LDFLAGS=$(RELEASE_LDFLAGS) -fprofile-use -fprofile-correction

.PHONY: all clean clean-profile debug release release-profiled setup

all: setup debug release-profiled

setup:
	@$(MKDIR) $(DEBUG_DIR)
	@$(MKDIR) $(RELEASE_DIR)
	@$(MKDIR) $(DEBUG_DEP_DIR)
	@$(MKDIR) $(RELEASE_DEP_DIR)

debug: setup $(DEBUG_EXEC)

release: setup $(RELEASE_EXEC)

release-profiled:
	@echo "Cleaning..."
	@$(MAKE) clean
	@$(MAKE) clean-profile
	@echo "Building with profile generation..."
	@$(MAKE) release CFLAGS="$(PROFILING_CFLAGS)" LDFLAGS="$(PROFILING_LDFLAGS)"
	@echo "Gathering profile data..."
	$(RELEASE_EXEC) run profile/profile.sl
	@echo "Building final version with profile data..."
	@$(MAKE) clean
	@$(MAKE) release CFLAGS="$(FINAL_CFLAGS)" LDFLAGS="$(FINAL_LDFLAGS)"

$(DEBUG_EXEC): $(DEBUG_OBJECTS)
	@$(LD) $^ -o $@ $(DEBUG_LDFLAGS) $(LIBS)

$(RELEASE_EXEC): $(RELEASE_OBJECTS)
	@$(LD) $^ -o $@ $(RELEASE_LDFLAGS) $(LIBS)

$(DEBUG_DIR)%.o: %.c
	@$(CC) $(DEBUG_CFLAGS) $(DEBUG_DEP_CFLAGS) -c $< -o $@

$(RELEASE_DIR)%.o: %.c
	@$(CC) $(RELEASE_CFLAGS) $(RELEASE_DEP_CFLAGS) -c $< -o $@

# Include dependencies if they exist
-include $(wildcard $(DEBUG_DEP_DIR)/*.d)
-include $(wildcard $(RELEASE_DEP_DIR)/*.d)

clean:
	@$(RM) $(DEBUG_OBJECTS) $(DEBUG_EXEC)
	@$(RM) $(RELEASE_OBJECTS) $(RELEASE_EXEC)
	@$(RM) -r $(DEBUG_DEP_DIR)
	@$(RM) -r $(RELEASE_DEP_DIR)

clean-profile:
	@$(RM) $(RELEASE_OBJECTS:.o=.gcda)
