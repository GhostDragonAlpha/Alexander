# Auto-generated execution command
# This file executes the test environment builder
import sys
import os

# Ensure path is correct
script_dir = os.path.dirname(os.path.abspath(__file__))
if script_dir not in sys.path:
    sys.path.insert(0, script_dir)

# Execute the auto-build script
exec(open(os.path.join(script_dir, 'auto_build_test_env.py')).read())
