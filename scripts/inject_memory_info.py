#!/usr/bin/env python3
"""
Script to inject memory usage info into firmware after compilation.
Parses PlatformIO build output and injects real flash usage.
"""

import os
import re
import sys
import subprocess
from pathlib import Path

def get_flash_usage_from_build():
    """Get actual flash usage from PlatformIO build output."""
    try:
        # Run platformio run --target size to get memory info
        result = subprocess.run(['platformio', 'run', '--target', 'size'], 
                              capture_output=True, text=True, cwd=os.getcwd())
        
        if result.returncode != 0:
            print(f"Error running platformio: {result.stderr}")
            return None
            
        output = result.stdout
        
        # Parse flash usage from output like:
        # Program:   29626 bytes (90.4% Full)
        flash_match = re.search(r'Program:\s+(\d+)\s+bytes', output)
        if flash_match:
            return int(flash_match.group(1))
            
        return None
        
    except Exception as e:
        print(f"Error getting flash usage: {e}")
        return None

def inject_flash_info():
    """Inject real flash usage into the firmware."""
    flash_used = get_flash_usage_from_build()
    if flash_used is None:
        print("Could not determine flash usage")
        return False
        
    flash_total = 30720  # 30KB (32KB - 2KB bootloader)
    flash_free = flash_total - flash_used
    
    print(f"Flash used: {flash_used} bytes")
    print(f"Flash free: {flash_free} bytes")
    print(f"Flash usage: {flash_used * 100 // flash_total}%")
    
    # Create a header file with the real values
    header_content = f"""#ifndef BUILD_MEMORY_INFO_H
#define BUILD_MEMORY_INFO_H

// Auto-generated memory info from build
#define BUILD_FLASH_USED {flash_used}
#define BUILD_FLASH_FREE {flash_free}
#define BUILD_FLASH_TOTAL {flash_total}

#endif
"""
    
    with open('src/BuildMemoryInfo.h', 'w') as f:
        f.write(header_content)
    
    print("Created src/BuildMemoryInfo.h with real memory info")
    
    # Force recompilation by touching a source file
    print("Forcing recompilation...")
    subprocess.run(['touch', 'src/main.cpp'], cwd=os.getcwd())
    
    return True

if __name__ == "__main__":
    inject_flash_info()
