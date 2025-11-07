# Basic Makefile for IoT-data-server

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
INCLUDES = -I./src/include

# Collect all .c files under src
SRC = $(shell find src -name '*.c')
# Object files go to build/ preserving directory structure
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

TARGET = iot-data-server

# Use pkg-config for dbus if available
PKG_CFLAGS_DBUS := $(shell pkg-config --cflags dbus-1 2>/dev/null)
PKG_LIBS_DBUS   := $(shell pkg-config --libs dbus-1 2>/dev/null)

# Libraries
LIBS = $(PKG_LIBS_DBUS) -lsqlite3

CFLAGS += $(INCLUDES) $(PKG_CFLAGS_DBUS)

.PHONY: all clean

all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Compile .c -> build/*.o
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
	rm -f $(TARGET)

# Print a helpful message if dbus pkg-config not found
ifeq ($(PKG_LIBS_DBUS),)
$(warning pkg-config for dbus-1 not found; make sure libdbus-1-dev is installed)
endif