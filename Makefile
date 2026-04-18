
CC = gcc
SRC_DIR = ./src
SRC_LIB = $(SRC_DIR)/webreq.c
SRC = $(SRC_DIR)/main.c
CFLAGS_COMMON = -Wall -Werror
CFLAGS_COMPILE = -c

CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2 -DNDEBUG

# Auto-detect OS
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    EXT = .exe
    LIB_EXT = .dll
    LIBS = -lws2_32
    MKDIR = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
else
    PLATFORM = linux
    EXT =
    LIBS =
    LIB_EXT = .so
    MKDIR = mkdir -p $1
endif

DEBUG_DIR = ./bin/debug/$(PLATFORM)
RELEASE_DIR = ./bin/release/$(PLATFORM)

lib_debug: $(DEBUG_DIR)
	$(CC) $(CFLAGS_DEBUG) $(CFLAGS_COMPILE) -fPIC $(SRC_LIB) -o $(DEBUG_DIR)/webreq.o	
	$(CC) -shared -o $(DEBUG_DIR)/libwebreq$(LIB_EXT) $(DEBUG_DIR)/webreq.o $(LIBS)

debug: lib_debug
	$(CC) $(CFLAGS_DEBUG) $(CFLAGS_COMPILE) -I$(SRC_DIR) $(SRC) -o $(DEBUG_DIR)/req.o $(LIBS)
	$(CC) $(DEBUG_DIR)/req.o -L$(DEBUG_DIR) -Wl,-rpath,$(DEBUG_DIR) -lwebreq -o $(DEBUG_DIR)/req$(EXT)

lib_release: $(RELEASE_DIR)
	$(CC) $(CFLAGS_RELEASE) $(CFLAGS_COMPILE) -fPIC $(SRC_LIB) -o $(RELEASE_DIR)/webreq.o
	$(CC) -shared -o $(RELEASE_DIR)/libwebreq$(LIB_EXT) $(RELEASE_DIR)/webreq.o $(LIBS)

release: lib_release
	$(CC) $(CFLAGS_RELEASE) $(CFLAGS_COMPILE) -I$(SRC_DIR) $(SRC) -o $(RELEASE_DIR)/req.o $(LIBS)
	$(CC) $(RELEASE_DIR)/req.o -L$(RELEASE_DIR) -Wl,-rpath,$(RELEASE_DIR) -lwebreq -o $(RELEASE_DIR)/req$(EXT)

build: debug


$(DEBUG_DIR):
	$(call MKDIR,$(DEBUG_DIR))

$(RELEASE_DIR):
	$(call MKDIR,$(RELEASE_DIR))

clean:
	@echo "Limpando arquivos compilados..."
	@rm -rf ./bin
	@echo "Limpeza concluída"

.PHONY: debug lib_debug release lib_release build clean