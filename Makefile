
CC = gcc
SRC = ./src/main.c
CFLAGS_COMMON = -Wall

CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2 -DNDEBUG

# Auto-detect OS
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    EXT = .exe
    LIBS = -lws2_32
    MKDIR = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
else
    PLATFORM = linux
    EXT =
    LIBS =
    MKDIR = mkdir -p $1
endif

DEBUG_DIR = ./bin/debug/$(PLATFORM)
RELEASE_DIR = ./bin/release/$(PLATFORM)

debug: | $(DEBUG_DIR)
	$(CC) $(CFLAGS_DEBUG) $(SRC) -o $(DEBUG_DIR)/req$(EXT) $(LIBS)

release: | $(RELEASE_DIR)
	$(CC) $(CFLAGS_RELEASE) $(SRC) -o $(RELEASE_DIR)/req$(EXT) $(LIBS)

build: debug

$(DEBUG_DIR):
	$(call MKDIR,$(DEBUG_DIR))

$(RELEASE_DIR):
	$(call MKDIR,$(RELEASE_DIR))