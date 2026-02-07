# MT25057
# PA02: Analysis of Network I/O primitives using "perf" tool
# Author: Aayush Amritesh (MT25057)

## Overview

This project implements and compares three different network I/O mechanisms for TCP socket communication:

1. **Two-Copy (A1)**: Standard `send()/recv()` socket primitives
2. **One-Copy (A2)**: `sendmsg()/recvmsg()` with scatter-gather I/O (iovec)
3. **Zero-Copy (A3)**: `sendmsg()` with `MSG_ZEROCOPY` flag

## File Structure

```
MT25057_PA02/
├── MT25057_Part_A1_Server.c          # Two-copy server implementation
├── MT25057_Part_A1_Client.c          # Two-copy client implementation
├── MT25057_Part_A2_Server.c          # One-copy server implementation
├── MT25057_Part_A2_Client.c          # One-copy client implementation
├── MT25057_Part_A3_Server.c          # Zero-copy server implementation
├── MT25057_Part_A3_Client.c          # Zero-copy client implementation
├── Makefile                          # Build automation
├── MT25057_Part_C_Experiment.sh      # Automated experiment script
├── MT25057_Part_D_Plot_Throughput.py # Throughput vs message size plot
├── MT25057_Part_D_Plot_Latency.py    # Latency vs thread count plot
├── MT25057_Part_D_Plot_CacheMisses.py # Cache misses vs message size plot
├── MT25057_Part_D_Plot_CPUCycles.py  # CPU cycles per byte plot
├── MT25057_Part_B_Results.csv        # Main experiment results (generated)
├── MT25057_Part_B_Perf.csv           # Perf profiling results (generated)
└── README.md                         # This file
```

## Prerequisites

- GCC compiler
- pthread library
- Linux kernel 4.14+ (for MSG_ZEROCOPY support)
- perf tools (`linux-tools-generic`)
- Python 3 with matplotlib
- netcat (nc)
- bc (basic calculator)

### Installation on Pop!_OS / Ubuntu

```bash
# Install build tools and perf
sudo apt-get update
sudo apt-get install build-essential linux-tools-generic linux-tools-$(uname -r)

# Install Python matplotlib
pip3 install matplotlib numpy

# Install utilities
sudo apt-get install netcat-openbsd bc
```

## Building

```bash
# Build all implementations
make all

# Build specific implementation
make a1  # Two-copy only
make a2  # One-copy only
make a3  # Zero-copy only

# Clean build artifacts
make clean
```

## Running

### Manual Execution

**Terminal 1 (Server):**
```bash
# Two-copy server
./MT25057_Part_A1_Server -p 8081 -s 4096

# One-copy server
./MT25057_Part_A2_Server -p 8082 -s 4096

# Zero-copy server
./MT25057_Part_A3_Server -p 8083 -s 4096
```

**Terminal 2 (Client):**
```bash
# Two-copy client
./MT25057_Part_A1_Client -h 127.0.0.1 -p 8081 -t 4 -d 10 -s 4096

# One-copy client
./MT25057_Part_A2_Client -h 127.0.0.1 -p 8082 -t 4 -d 10 -s 4096

# Zero-copy client
./MT25057_Part_A3_Client -h 127.0.0.1 -p 8083 -t 4 -d 10 -s 4096
```

### Command Line Options

**Server:**
- `-p port`: Server port (default: 8081/8082/8083)
- `-s size`: Message size in bytes (default: 1024)

**Client:**
- `-h host`: Server hostname (default: 127.0.0.1)
- `-p port`: Server port
- `-t threads`: Number of client threads (default: 1)
- `-d duration`: Test duration in seconds (default: 10)
- `-s size`: Message size in bytes (default: 1024)

### Automated Experiments

```bash
# Make script executable
chmod +x MT25057_Part_C_Experiment.sh

# Run all experiments (requires sudo for perf)
sudo ./MT25057_Part_C_Experiment.sh
```

This will:
1. Compile all implementations
2. Run experiments with various message sizes (256B, 1KB, 4KB, 16KB, 64KB)
3. Run experiments with various thread counts (1, 2, 4, 8)
4. Collect perf statistics (CPU cycles, cache misses, context switches)
5. Generate CSV files with results

## Generating Plots

After running experiments, update the hardcoded values in the plotting scripts with actual data, then run:

```bash
python3 MT25057_Part_D_Plot_Throughput.py
python3 MT25057_Part_D_Plot_Latency.py
python3 MT25057_Part_D_Plot_CacheMisses.py
python3 MT25057_Part_D_Plot_CPUCycles.py
```

## Implementation Details

### A1: Two-Copy Implementation
- Uses standard `send()` and `recv()` system calls
- Data flow: User buffer → Kernel socket buffer → NIC
- Two copies occur: one from user to kernel, one from kernel to NIC (via DMA)

### A2: One-Copy Implementation
- Uses `sendmsg()` with scatter-gather I/O (iovec)
- Eliminates the need to serialize message fields into a contiguous buffer
- Data is gathered from multiple user-space buffers directly by the kernel
- One copy eliminated: User-space buffer serialization

### A3: Zero-Copy Implementation
- Uses `sendmsg()` with `MSG_ZEROCOPY` flag
- Kernel pins user-space pages and DMAs directly from them
- Requires completion notification handling via error queue
- Eliminates kernel socket buffer copy for large messages

## Performance Metrics

The experiments measure:
- **Throughput** (Gbps): Data transfer rate
- **Latency** (μs): Time per message
- **CPU Cycles**: Total CPU cycles consumed
- **L1 Cache Misses**: First-level cache misses
- **LLC Cache Misses**: Last-level (L3) cache misses
- **Context Switches**: Number of context switches

## Troubleshooting

1. **MSG_ZEROCOPY not supported**: Requires Linux kernel 4.14+
2. **Permission denied for perf**: Run with `sudo`
3. **Port already in use**: Wait or use different port
4. **perf not found**: Install `linux-tools-generic` and `linux-tools-$(uname -r)`

---

This code was generated with the assistance of Claude Opus 4.5 by Anthropic.
