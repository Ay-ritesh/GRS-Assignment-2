#!/bin/bash
# MT25057 - Quick Experiment Runner
# Run experiments without requiring sudo for perf

cd /home/ay-ritesh/Documents/GRS/MT25057_PA02

# Configuration
MESSAGE_SIZES=(256 1024 4096 16384 65536)
THREAD_COUNTS=(1 2 4 8)
DURATION=5

CSV_MAIN="MT25057_Part_B_Results.csv"

# Clean up any previous processes
pkill -f "MT25057_Part" 2>/dev/null || true
sleep 1

# Initialize CSV
echo "implementation,threads,msg_size,throughput_gbps,latency_us,bytes_total,elapsed_s" > "$CSV_MAIN"

run_test() {
    local impl=$1
    local impl_num=$2
    local port=$3
    local msg_size=$4
    local threads=$5
    
    echo "Running: $impl, msg_size=$msg_size, threads=$threads"
    
    # Start server
    ./MT25057_Part_${impl_num}_Server -p $port -s $msg_size > /dev/null 2>&1 &
    local server_pid=$!
    sleep 1
    
    # Check server started
    if ! nc -z localhost $port 2>/dev/null; then
        echo "  Server failed to start"
        kill $server_pid 2>/dev/null || true
        return 1
    fi
    
    # Run client and capture output
    local output=$(./MT25057_Part_${impl_num}_Client -h 127.0.0.1 -p $port -t $threads -d $DURATION -s $msg_size 2>&1)
    
    # Extract CSV line
    local csv_line=$(echo "$output" | grep "^${impl}," | tail -1)
    if [ -n "$csv_line" ]; then
        echo "$csv_line" >> "$CSV_MAIN"
        echo "  Result: $csv_line"
    else
        echo "  No result captured"
    fi
    
    # Stop server
    kill $server_pid 2>/dev/null || true
    wait $server_pid 2>/dev/null || true
    sleep 0.5
}

echo "Starting experiments..."
echo ""

for msg_size in "${MESSAGE_SIZES[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        run_test "two_copy" "A1" 8081 $msg_size $threads
        run_test "one_copy" "A2" 8082 $msg_size $threads
        run_test "zero_copy" "A3" 8083 $msg_size $threads
        echo ""
    done
done

echo "Experiments complete!"
echo ""
echo "Results in: $CSV_MAIN"
cat "$CSV_MAIN"
