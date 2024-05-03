CC=gcc
LD=gcc
RM=rm -f
MKDIR=mkdir -p

# General compiler flags
CFLAGS=-Wall -Wextra -Werror -std=c17 -m64 -D_UNICODE -DUNICODE
LDFLAGS=-m64
LIBS=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32

# Source files and output
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
DEBUG_EXEC=bin/x64/debug/slang.exe
RELEASE_EXEC=bin/x64/release/slang.exe
DEBUG_DIR=bin/x64/debug/
RELEASE_DIR=bin/x64/release/
DEBUG_OBJECTS=$(addprefix $(DEBUG_DIR), $(OBJECTS))
RELEASE_OBJECTS=$(addprefix $(RELEASE_DIR), $(OBJECTS))

# Debug specific flags
DEBUG_CFLAGS=$(CFLAGS) -g -O0 -D_DEBUG
DEBUG_LDFLAGS=$(LDFLAGS) -g

# Release specific flags
RELEASE_CFLAGS=$(CFLAGS) -O3 -DNDEBUG -flto=8 # Arbirary, could be retrieved with $(shell nproc) on linux or $(NUMBER_OF_PROCESSORS) on windows
RELEASE_LDFLAGS=$(LDFLAGS) -flto

# Experimental optimization flags for release builds
# RELEASE_CFLAGS += -march=native -funroll_loops

.PHONY: all clean debug release setup

all: setup debug release

setup:
	@$(MKDIR) $(DEBUG_DIR)
	@$(MKDIR) $(RELEASE_DIR)

debug: setup $(DEBUG_EXEC)

release: setup $(RELEASE_EXEC)

$(DEBUG_EXEC): $(DEBUG_OBJECTS)
	$(LD) $^ -o $@ $(DEBUG_LDFLAGS) $(LIBS)

$(RELEASE_EXEC): $(RELEASE_OBJECTS)
	$(LD) $^ -o $@ $(RELEASE_LDFLAGS) $(LIBS)

$(DEBUG_DIR)%.o: %.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

$(RELEASE_DIR)%.o: %.c
	$(CC) $(RELEASE_CFLAGS) -c $< -o $@

clean:
	@$(RM) $(DEBUG_OBJECTS) $(DEBUG_EXEC)
	@$(RM) $(RELEASE_OBJECTS) $(RELEASE_EXEC)
