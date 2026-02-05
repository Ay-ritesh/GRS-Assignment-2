#!/usr/bin/env python3
"""
MT25057
PA02: Analysis of Network I/O primitives using "perf" tool
Part D: Plotting - Latency vs Thread Count

Author: Aayush Amritesh (MT25057)

This script generates a plot of latency (microseconds) vs thread count
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

# Thread counts
thread_counts = [1, 2, 4, 8]

# Latency values in microseconds (from actual experiments)
# Format: [value_for_1_thread, value_for_2_threads, value_for_4_threads, value_for_8_threads]
# Using message size = 4096 bytes as reference

# Two-Copy (send/recv) latency
two_copy_latency = [4.07, 4.07, 4.33, 5.14]

# One-Copy (sendmsg with iovec) latency
one_copy_latency = [4.18, 4.26, 4.56, 6.32]

# Zero-Copy (MSG_ZEROCOPY) latency
zero_copy_latency = [4.57, 4.97, 5.75, 6.92]

# Create the plot
fig, ax = plt.subplots(figsize=(10, 6))

# Plot data with markers
ax.plot(thread_counts, two_copy_latency, 'o-', label='Two-Copy (send/recv)', 
        linewidth=2, markersize=8, color='#1f77b4')
ax.plot(thread_counts, one_copy_latency, 's-', label='One-Copy (sendmsg)', 
        linewidth=2, markersize=8, color='#ff7f0e')
ax.plot(thread_counts, zero_copy_latency, '^-', label='Zero-Copy (MSG_ZEROCOPY)', 
        linewidth=2, markersize=8, color='#2ca02c')

# Labels and title
ax.set_xlabel('Thread Count', fontsize=12)
ax.set_ylabel('Latency (μs)', fontsize=12)
ax.set_title('Latency vs Thread Count (Message Size = 4KB)\n(MT25057 - PA02)', fontsize=14)

# Grid
ax.grid(True, alpha=0.3, linestyle='--')

# Legend
ax.legend(loc='upper left', fontsize=10)

# Set x-axis ticks
ax.set_xticks(thread_counts)
ax.set_xticklabels(['1', '2', '4', '8'])

# Add system configuration text
props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)
ax.text(0.98, 0.02, SYSTEM_CONFIG.strip(), transform=ax.transAxes, fontsize=8,
        verticalalignment='bottom', horizontalalignment='right', bbox=props)

# Tight layout
plt.tight_layout()

# Save the plot
plt.savefig('MT25057_Part_D_Latency_vs_Threads.pdf', dpi=300, bbox_inches='tight')
plt.savefig('MT25057_Part_D_Latency_vs_Threads.png', dpi=300, bbox_inches='tight')

print("Plot saved: MT25057_Part_D_Latency_vs_Threads.pdf/png")

# plt.show()  # Commented out for headless execution
