#!/usr/bin/env python3
"""
MT25057
PA02: Analysis of Network I/O primitives using "perf" tool
Part D: Plotting - Throughput vs Message Size

Author: Aayush Amritesh (MT25057)

This script generates a plot of throughput (Gbps) vs message size
for all three implementations (two_copy, one_copy, zero_copy).

NOTE: Values are hardcoded as per assignment requirements.
Update these values after running experiments.
"""

import matplotlib.pyplot as plt
import numpy as np

# System configuration (update based on your system)
SYSTEM_CONFIG = """
System: Pop!_OS 24.04 LTS
CPU: AMD Ryzen 7 7840HS
RAM: 13GB DDR5
"""

# Message sizes in bytes
message_sizes = [256, 1024, 4096, 16384, 65536]

# Throughput values in Gbps (from actual experiments)
# Format: [value_for_256B, value_for_1KB, value_for_4KB, value_for_16KB, value_for_64KB]

# Two-Copy (send/recv) throughput - Thread count = 4
two_copy_throughput = [3.10, 7.07, 29.82, 87.65, 142.86]

# One-Copy (sendmsg with iovec) throughput - Thread count = 4
one_copy_throughput = [2.61, 8.97, 28.38, 82.80, 139.67]

# Zero-Copy (MSG_ZEROCOPY) throughput - Thread count = 4
# Note: Zero-copy shows poor performance for small messages due to overhead
zero_copy_throughput = [0.02, 0.0002, 22.53, 62.12, 115.60]

# Create the plot
fig, ax = plt.subplots(figsize=(10, 6))

# Plot data with markers
ax.plot(message_sizes, two_copy_throughput, 'o-', label='Two-Copy (send/recv)', 
        linewidth=2, markersize=8, color='#1f77b4')
ax.plot(message_sizes, one_copy_throughput, 's-', label='One-Copy (sendmsg)', 
        linewidth=2, markersize=8, color='#ff7f0e')
ax.plot(message_sizes, zero_copy_throughput, '^-', label='Zero-Copy (MSG_ZEROCOPY)', 
        linewidth=2, markersize=8, color='#2ca02c')

# Set logarithmic scale for x-axis
ax.set_xscale('log', base=2)

# Labels and title
ax.set_xlabel('Message Size (bytes)', fontsize=12)
ax.set_ylabel('Throughput (Gbps)', fontsize=12)
ax.set_title('Throughput vs Message Size\n(MT25057 - PA02)', fontsize=14)

# Grid
ax.grid(True, alpha=0.3, linestyle='--')

# Legend
ax.legend(loc='upper left', fontsize=10)

# Set x-axis ticks
ax.set_xticks(message_sizes)
ax.set_xticklabels(['256B', '1KB', '4KB', '16KB', '64KB'])

# Add system configuration text
props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)
ax.text(0.98, 0.02, SYSTEM_CONFIG.strip(), transform=ax.transAxes, fontsize=8,
        verticalalignment='bottom', horizontalalignment='right', bbox=props)

# Tight layout
plt.tight_layout()

# Save the plot
plt.savefig('MT25057_Part_D_Throughput_vs_MsgSize.pdf', dpi=300, bbox_inches='tight')
plt.savefig('MT25057_Part_D_Throughput_vs_MsgSize.png', dpi=300, bbox_inches='tight')

print("Plot saved: MT25057_Part_D_Throughput_vs_MsgSize.pdf/png")

# plt.show()  # Commented out for headless execution
