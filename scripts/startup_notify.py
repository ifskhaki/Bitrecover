#!/usr/bin/env python3
"""
Startup notification script - called by Bitrecover on startup
"""

import sys
import os

# Add parent directory to path to import send_email
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from send_email import send_startup_notification

if __name__ == "__main__":
    # Parse arguments from C++ call
    # Format: hostname os gpu_count [gpu_name memory_mb]...
    
    if len(sys.argv) < 4:
        print("Usage: python startup_notify.py <hostname> <os> <gpu_count> [gpu_name memory_mb]...")
        sys.exit(1)
    
    sys_info = {
        'hostname': sys.argv[1],
        'os': sys.argv[2],
        'gpu_count': int(sys.argv[3]),
        'gpus': []
    }
    
    # Parse GPU information
    idx = 4
    for i in range(sys_info['gpu_count']):
        if idx + 1 < len(sys.argv):
            gpu = {
                'name': sys.argv[idx],
                'memory_mb': int(sys.argv[idx + 1])
            }
            sys_info['gpus'].append(gpu)
            idx += 2
    
    send_startup_notification(sys_info)

