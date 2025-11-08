"""
INSTANT VALIDATION EXECUTION
Run this in Unreal Python console for immediate validation
"""
import unreal
import json
import os
from datetime import datetime

print("=" * 60)
print("EXECUTING AUTONOMOUS VALIDATION NOW")
print("=" * 60)

# Execute validation
try:
    exec(open('Content/Python/automated_validation_unreal.py').read())
    print("VALIDATION EXECUTION COMPLETE")
except Exception as e:
    print(f"VALIDATION ERROR: {e}")

# Take screenshot
try:
    exec(open('Content/Python/snap.py').read())
    print("SCREENSHOT TAKEN")
except Exception as e:
    print(f"SCREENSHOT ERROR: {e}")

print("=" * 60)
print("AUTONOMOUS EXECUTION FINISHED - Check output files")
print("=" * 60)
