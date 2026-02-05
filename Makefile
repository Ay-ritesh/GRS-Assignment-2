# MT25057
# PA02: Analysis of Network I/O primitives using "perf" tool
# Makefile for compiling all implementations
# Author: Aayush Amritesh (MT25057)

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -pthread
LDFLAGS = -pthread

# Source files
A1_SERVER = MT25057_Part_A1_Server.c
A1_CLIENT = MT25057_Part_A1_Client.c
A2_SERVER = MT25057_Part_A2_Server.c
A2_CLIENT = MT25057_Part_A2_Client.c
A3_SERVER = MT25057_Part_A3_Server.c
A3_CLIENT = MT25057_Part_A3_Client.c

# Binary outputs
A1_SERVER_BIN = MT25057_Part_A1_Server
A1_CLIENT_BIN = MT25057_Part_A1_Client
A2_SERVER_BIN = MT25057_Part_A2_Server
A2_CLIENT_BIN = MT25057_Part_A2_Client
A3_SERVER_BIN = MT25057_Part_A3_Server
A3_CLIENT_BIN = MT25057_Part_A3_Client

# All binaries
BINS = $(A1_SERVER_BIN) $(A1_CLIENT_BIN) \
       $(A2_SERVER_BIN) $(A2_CLIENT_BIN) \
       $(A3_SERVER_BIN) $(A3_CLIENT_BIN)

.PHONY: all clean a1 a2 a3

all: $(BINS)

# A1: Two-Copy Implementation
a1: $(A1_SERVER_BIN) $(A1_CLIENT_BIN)

$(A1_SERVER_BIN): $(A1_SERVER)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A1_CLIENT_BIN): $(A1_CLIENT)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# A2: One-Copy Implementation
a2: $(A2_SERVER_BIN) $(A2_CLIENT_BIN)

$(A2_SERVER_BIN): $(A2_SERVER)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A2_CLIENT_BIN): $(A2_CLIENT)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# A3: Zero-Copy Implementation
a3: $(A3_SERVER_BIN) $(A3_CLIENT_BIN)

$(A3_SERVER_BIN): $(A3_SERVER)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A3_CLIENT_BIN): $(A3_CLIENT)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(BINS)

# Help target
help:
	@echo "MT25057 PA02 - Network I/O Primitives"
	@echo "Available targets:"
	@echo "  all     - Build all implementations"
	@echo "  a1      - Build A1 (Two-Copy) implementation"
	@echo "  a2      - Build A2 (One-Copy) implementation"
	@echo "  a3      - Build A3 (Zero-Copy) implementation"
	@echo "  clean   - Remove all binaries"
	@echo "  help    - Show this help message"
