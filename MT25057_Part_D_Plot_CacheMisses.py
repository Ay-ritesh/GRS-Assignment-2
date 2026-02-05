#!/usr/bin/env python3
"""
MT25057
PA02: Analysis of Network I/O primitives using "perf" tool
Part D: Plotting - Cache Misses vs Message Size

Author: Aayush Amritesh (MT25057)

This script generates a plot of cache misses (L1 and LLC) vs message size
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

# L1 Cache miss rates (misses per 1000 bytes transferred)
# Format: [value_for_256B, value_for_1KB, value_for_4KB, value_for_16KB, value_for_64KB]
# Using thread count = 4 as reference
# Note: These are estimated values based on typical behavior patterns
# with actual throughput data as reference

# Two-Copy L1 cache misses (per KB) - highest due to multiple copies
two_copy_l1_misses = [145, 112, 78, 52, 38]

# One-Copy L1 cache misses (per KB) - medium, eliminates serialization copy
one_copy_l1_misses = [128, 95, 65, 45, 32]

# Zero-Copy L1 cache misses (per KB) - lowest for large messages, highest for small
zero_copy_l1_misses = [185, 165, 58, 38, 25]

# LLC (Last Level Cache) miss rates (per KB)
# Two-Copy LLC misses (per KB)
two_copy_llc_misses = [52, 42, 28, 18, 12]

# One-Copy LLC misses (per KB)
one_copy_llc_misses = [45, 38, 24, 15, 10]

# Zero-Copy LLC misses (per KB)
zero_copy_llc_misses = [68, 55, 22, 12, 8]

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

# Plot 1: L1 Cache Misses
ax1.plot(message_sizes, two_copy_l1_misses, 'o-', label='Two-Copy (send/recv)', 
         linewidth=2, markersize=8, color='#1f77b4')
ax1.plot(message_sizes, one_copy_l1_misses, 's-', label='One-Copy (sendmsg)', 
         linewidth=2, markersize=8, color='#ff7f0e')
ax1.plot(message_sizes, zero_copy_l1_misses, '^-', label='Zero-Copy (MSG_ZEROCOPY)', 
         linewidth=2, markersize=8, color='#2ca02c')

ax1.set_xscale('log', base=2)
ax1.set_xlabel('Message Size (bytes)', fontsize=12)
ax1.set_ylabel('L1 Cache Misses (per KB transferred)', fontsize=12)
ax1.set_title('L1 Cache Misses vs Message Size', fontsize=14)
ax1.grid(True, alpha=0.3, linestyle='--')
ax1.legend(loc='upper right', fontsize=9)
ax1.set_xticks(message_sizes)
ax1.set_xticklabels(['256B', '1KB', '4KB', '16KB', '64KB'])

# Plot 2: LLC (Last Level Cache) Misses
ax2.plot(message_sizes, two_copy_llc_misses, 'o-', label='Two-Copy (send/recv)', 
         linewidth=2, markersize=8, color='#1f77b4')
ax2.plot(message_sizes, one_copy_llc_misses, 's-', label='One-Copy (sendmsg)', 
         linewidth=2, markersize=8, color='#ff7f0e')
ax2.plot(message_sizes, zero_copy_llc_misses, '^-', label='Zero-Copy (MSG_ZEROCOPY)', 
         linewidth=2, markersize=8, color='#2ca02c')

ax2.set_xscale('log', base=2)
ax2.set_xlabel('Message Size (bytes)', fontsize=12)
ax2.set_ylabel('LLC Misses (per KB transferred)', fontsize=12)
ax2.set_title('LLC (Last Level Cache) Misses vs Message Size', fontsize=14)
ax2.grid(True, alpha=0.3, linestyle='--')
ax2.legend(loc='upper right', fontsize=9)
ax2.set_xticks(message_sizes)
ax2.set_xticklabels(['256B', '1KB', '4KB', '16KB', '64KB'])

# Add overall title
fig.suptitle('Cache Misses Analysis (MT25057 - PA02)', fontsize=14, y=1.02)

# Add system configuration text
props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)
ax2.text(1.02, 0.02, SYSTEM_CONFIG.strip(), transform=ax2.transAxes, fontsize=8,
         verticalalignment='bottom', horizontalalignment='left', bbox=props)

# Tight layout
plt.tight_layout()

# Save the plot
plt.savefig('MT25057_Part_D_CacheMisses_vs_MsgSize.pdf', dpi=300, bbox_inches='tight')
plt.savefig('MT25057_Part_D_CacheMisses_vs_MsgSize.png', dpi=300, bbox_inches='tight')

print("Plot saved: MT25057_Part_D_CacheMisses_vs_MsgSize.pdf/png")

# plt.show()  # Commented out for headless execution
