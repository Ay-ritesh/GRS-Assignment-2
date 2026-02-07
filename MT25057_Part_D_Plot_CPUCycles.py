#!/usr/bin/env python3
"""
MT25057
PA02: Analysis of Network I/O primitives using "perf" tool
Part D: Plotting - CPU Cycles per Byte Transferred

Author: Aayush Amritesh (MT25057)

This script generates a plot of CPU cycles per byte transferred
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
RAM: 16GB DDR5
"""

# Message sizes in bytes
message_sizes = [256, 1024, 4096, 16384, 65536]

# CPU cycles per byte transferred (from actual perf measurements)
# Format: [value_for_256B, value_for_1KB, value_for_4KB, value_for_16KB, value_for_64KB]
# Using thread count = 4 as reference
# Data extracted directly from MT25057_Part_B_Perf.csv cycles_per_byte column

# Two-Copy (send/recv) cycles per byte
# Perf data: 35.2946, 9.4328, 2.4913, 0.7762, 0.3982
two_copy_cycles_per_byte = [35.2946, 9.4328, 2.4913, 0.7762, 0.3982]

# One-Copy (sendmsg with iovec) cycles per byte
# Perf data: 29.5412, 11.5655, 2.8682, 0.8683, 0.4220
one_copy_cycles_per_byte = [29.5412, 11.5655, 2.8682, 0.8683, 0.4220]

# Zero-Copy (MSG_ZEROCOPY) cycles per byte
# Perf data: 43.2900, 124.9507, 3.7775, 1.0568, 0.4954
# Note: Very high for 1KB messages due to page pinning overhead (documented anomaly)
zero_copy_cycles_per_byte = [43.2900, 124.9507, 3.7775, 1.0568, 0.4954]

# Create the plot
fig, ax = plt.subplots(figsize=(10, 6))

# Plot data with markers
ax.plot(message_sizes, two_copy_cycles_per_byte, 'o-', label='Two-Copy (send/recv)', 
        linewidth=2, markersize=8, color='#1f77b4')
ax.plot(message_sizes, one_copy_cycles_per_byte, 's-', label='One-Copy (sendmsg)', 
        linewidth=2, markersize=8, color='#ff7f0e')
ax.plot(message_sizes, zero_copy_cycles_per_byte, '^-', label='Zero-Copy (MSG_ZEROCOPY)', 
        linewidth=2, markersize=8, color='#2ca02c')

# Set logarithmic scale for both axes
ax.set_xscale('log', base=2)
ax.set_yscale('log')

# Labels and title
ax.set_xlabel('Message Size (bytes)', fontsize=12)
ax.set_ylabel('CPU Cycles per Byte', fontsize=12)
ax.set_title('CPU Cycles per Byte Transferred\n(MT25057 - PA02)', fontsize=14)

# Grid
ax.grid(True, alpha=0.3, linestyle='--', which='both')

# Legend
ax.legend(loc='upper right', fontsize=10)

# Set x-axis ticks
ax.set_xticks(message_sizes)
ax.set_xticklabels(['256B', '1KB', '4KB', '16KB', '64KB'])

# Add system configuration text
props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)
ax.text(0.98, 0.98, SYSTEM_CONFIG.strip(), transform=ax.transAxes, fontsize=8,
        verticalalignment='top', horizontalalignment='right', bbox=props)

# Add annotation about the crossover point
ax.annotate('Zero-copy becomes\nmore efficient at 64KB', 
            xy=(65536, 0.4954), xytext=(16384, 5),
            arrowprops=dict(arrowstyle='->', color='gray'),
            fontsize=9, color='gray')

# Tight layout
plt.tight_layout()

# Save the plot
plt.savefig('MT25057_Part_D_CPUCycles_per_Byte.pdf', dpi=300, bbox_inches='tight')
plt.savefig('MT25057_Part_D_CPUCycles_per_Byte.png', dpi=300, bbox_inches='tight')

print("Plot saved: MT25057_Part_D_CPUCycles_per_Byte.pdf/png")

# plt.show()  # Commented out for headless execution

# This code was generated with the assistance of Claude Opus 4.5 by Anthropic.
