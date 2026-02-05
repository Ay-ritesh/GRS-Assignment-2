#!/bin/bash
# MT25057
# PA02: Analysis of Network I/O primitives using "perf" tool
# Automated Experiment Script
# Author: Aayush Amritesh (MT25057)
#
# This script:
# 1. Compiles all implementations
# 2. Runs experiments across message sizes and thread counts
# 3. Collects profiling output automatically using perf stat
# 4. Stores results in CSV format
#
# Usage: ./MT25057_Part_C_Experiment.sh
#

set -e  # Exit on error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Experiment parameters - at least 4 distinct values each
MESSAGE_SIZES=(256 1024 4096 16384 65536)  # bytes
THREAD_COUNTS=(1 2 4 8)
DURATION=5  # seconds per experiment

# Ports for each implementation
PORT_A1=8081
PORT_A2=8082
PORT_A3=8083

# Output CSV files
CSV_MAIN="MT25057_Part_B_Results.csv"
CSV_PERF="MT25057_Part_B_Perf.csv"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to clean up background processes
cleanup() {
    log_info "Cleaning up..."
    # Kill any remaining server processes
    pkill -f "MT25057_Part_A[123]_Server" 2>/dev/null || true
    wait 2>/dev/null || true
}

trap cleanup EXIT

# Function to wait for server to be ready
wait_for_server() {
    local port=$1
    local max_attempts=30
    local attempt=0
    
    while ! nc -z localhost $port 2>/dev/null; do
        attempt=$((attempt + 1))
        if [ $attempt -ge $max_attempts ]; then
            log_error "Server on port $port did not start in time"
            return 1
        fi
        sleep 0.1
    done
    return 0
}

# Function to run a single experiment
run_experiment() {
    local impl=$1       # Implementation name (two_copy, one_copy, zero_copy)
    local impl_num=$2   # A1, A2, A3
    local port=$3
    local msg_size=$4
    local threads=$5
    
    local server_bin="./MT25057_Part_${impl_num}_Server"
    local client_bin="./MT25057_Part_${impl_num}_Client"
    
    log_info "Running: $impl, msg_size=$msg_size, threads=$threads"
    
    # Start server in background
    $server_bin -p $port -s $msg_size > /dev/null 2>&1 &
    local server_pid=$!
    
    # Wait for server to be ready
    if ! wait_for_server $port; then
        kill $server_pid 2>/dev/null || true
        return 1
    fi
    
    # Create temporary file for client output
    local client_output=$(mktemp)
    local perf_output=$(mktemp)
    
    # Run client with perf stat
    perf stat -e cycles,instructions,cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses,context-switches \
        -o "$perf_output" \
        $client_bin -h 127.0.0.1 -p $port -t $threads -d $DURATION -s $msg_size > "$client_output" 2>&1 || true
    
    # Stop server
    kill $server_pid 2>/dev/null || true
    wait $server_pid 2>/dev/null || true
    
    # Parse client output for throughput and latency
    local throughput=$(grep "^${impl}," "$client_output" | tail -1 | cut -d',' -f4)
    local latency=$(grep "^${impl}," "$client_output" | tail -1 | cut -d',' -f5)
    local bytes_total=$(grep "^${impl}," "$client_output" | tail -1 | cut -d',' -f6)
    local elapsed=$(grep "^${impl}," "$client_output" | tail -1 | cut -d',' -f7)
    
    # Default values if parsing fails
    throughput=${throughput:-0}
    latency=${latency:-0}
    bytes_total=${bytes_total:-0}
    elapsed=${elapsed:-0}
    
    # Parse perf output
    local cycles=$(grep "cycles" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local instructions=$(grep "instructions" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local cache_refs=$(grep "cache-references" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local cache_misses=$(grep "cache-misses" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local l1_loads=$(grep "L1-dcache-loads" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local l1_misses=$(grep "L1-dcache-load-misses" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local llc_loads=$(grep "LLC-loads" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local llc_misses=$(grep "LLC-load-misses" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local ctx_switches=$(grep "context-switches" "$perf_output" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    
    # Default values
    cycles=${cycles:-0}
    instructions=${instructions:-0}
    cache_refs=${cache_refs:-0}
    cache_misses=${cache_misses:-0}
    l1_loads=${l1_loads:-0}
    l1_misses=${l1_misses:-0}
    llc_loads=${llc_loads:-0}
    llc_misses=${llc_misses:-0}
    ctx_switches=${ctx_switches:-0}
    
    # Calculate CPU cycles per byte
    local cycles_per_byte=0
    if [ "$bytes_total" != "0" ] && [ -n "$bytes_total" ]; then
        cycles_per_byte=$(echo "scale=4; $cycles / $bytes_total" | bc 2>/dev/null || echo "0")
    fi
    
    # Append to main CSV
    echo "$impl,$threads,$msg_size,$throughput,$latency,$bytes_total,$elapsed" >> "$CSV_MAIN"
    
    # Append to perf CSV
    echo "$impl,$threads,$msg_size,$cycles,$instructions,$cache_refs,$cache_misses,$l1_loads,$l1_misses,$llc_loads,$llc_misses,$ctx_switches,$cycles_per_byte" >> "$CSV_PERF"
    
    # Clean up temp files
    rm -f "$client_output" "$perf_output"
    
    # Small delay between experiments
    sleep 1
    
    return 0
}

# Main script

log_info "================================================"
log_info "PA02 Automated Experiment Script"
log_info "Author: Aayush Amritesh (MT25057)"
log_info "================================================"

# Check for required tools
log_info "Checking prerequisites..."

if ! command -v perf &> /dev/null; then
    log_warn "perf not found. Installing linux-tools..."
    sudo apt-get update && sudo apt-get install -y linux-tools-generic linux-tools-$(uname -r) || {
        log_error "Failed to install perf. Please install manually."
        exit 1
    }
fi

if ! command -v nc &> /dev/null; then
    log_info "Installing netcat..."
    sudo apt-get update && sudo apt-get install -y netcat-openbsd || true
fi

if ! command -v bc &> /dev/null; then
    log_info "Installing bc..."
    sudo apt-get update && sudo apt-get install -y bc || true
fi

# Step 1: Compile all implementations
log_info "Step 1: Compiling all implementations..."
make clean
make all

if [ $? -ne 0 ]; then
    log_error "Compilation failed!"
    exit 1
fi
log_info "Compilation successful!"

# Step 2: Initialize CSV files
log_info "Step 2: Initializing CSV files..."

echo "implementation,threads,msg_size,throughput_gbps,latency_us,bytes_total,elapsed_s" > "$CSV_MAIN"
echo "implementation,threads,msg_size,cycles,instructions,cache_refs,cache_misses,l1_loads,l1_misses,llc_loads,llc_misses,ctx_switches,cycles_per_byte" > "$CSV_PERF"

# Step 3: Run experiments
log_info "Step 3: Running experiments..."
log_info "Message sizes: ${MESSAGE_SIZES[*]}"
log_info "Thread counts: ${THREAD_COUNTS[*]}"
log_info "Duration per test: ${DURATION}s"

total_experiments=$(( ${#MESSAGE_SIZES[@]} * ${#THREAD_COUNTS[@]} * 3 ))
current_experiment=0

for msg_size in "${MESSAGE_SIZES[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        # A1: Two-Copy
        current_experiment=$((current_experiment + 1))
        log_info "Progress: $current_experiment / $total_experiments"
        run_experiment "two_copy" "A1" $PORT_A1 $msg_size $threads || true
        
        # A2: One-Copy
        current_experiment=$((current_experiment + 1))
        log_info "Progress: $current_experiment / $total_experiments"
        run_experiment "one_copy" "A2" $PORT_A2 $msg_size $threads || true
        
        # A3: Zero-Copy
        current_experiment=$((current_experiment + 1))
        log_info "Progress: $current_experiment / $total_experiments"
        run_experiment "zero_copy" "A3" $PORT_A3 $msg_size $threads || true
    done
done

# Step 4: Summary
log_info "================================================"
log_info "Experiment completed!"
log_info "Results saved to:"
log_info "  - $CSV_MAIN"
log_info "  - $CSV_PERF"
log_info "================================================"

# Display summary statistics
log_info "Summary of results:"
echo ""
echo "=== Main Results ==="
cat "$CSV_MAIN"
echo ""
echo "=== Perf Results (first 10 rows) ==="
head -11 "$CSV_PERF"
echo ""

log_info "Done!"
