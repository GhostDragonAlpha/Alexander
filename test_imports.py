#!/usr/bin/env python3
"""
Test script to debug import issues in continuous_playtesting.py
"""

print("Testing imports one by one...")

try:
    import json
    print("OK json imported")
except Exception as e:
    print(f"FAIL json failed: {e}")

try:
    import time
    print("OK time imported")
except Exception as e:
    print(f"FAIL time failed: {e}")

try:
    import logging
    print("OK logging imported")
except Exception as e:
    print(f"FAIL logging failed: {e}")

try:
    import signal
    print("OK signal imported")
except Exception as e:
    print(f"FAIL signal failed: {e}")

try:
    import sys
    print("OK sys imported")
except Exception as e:
    print(f"FAIL sys failed: {e}")

try:
    import argparse
    print("OK argparse imported")
except Exception as e:
    print(f"FAIL argparse failed: {e}")

try:
    import subprocess
    print("OK subprocess imported")
except Exception as e:
    print(f"FAIL subprocess failed: {e}")

try:
    from pathlib import Path
    print("OK Path imported")
except Exception as e:
    print(f"FAIL Path failed: {e}")

try:
    from typing import Dict, Any, List, Optional
    print("OK typing imports imported")
except Exception as e:
    print(f"FAIL typing imports failed: {e}")

try:
    from datetime import datetime
    print("OK datetime imported")
except Exception as e:
    print(f"FAIL datetime failed: {e}")

try:
    import threading
    print("OK threading imported")
except Exception as e:
    print(f"FAIL threading failed: {e}")

try:
    import requests
    print("OK requests imported")
except Exception as e:
    print(f"FAIL requests failed: {e}")

try:
    import psutil
    print("OK psutil imported")
except Exception as e:
    print(f"FAIL psutil failed: {e}")

print("\nTesting component imports...")

try:
    from launch_game_desktop import GameLauncher
    print("OK GameLauncher imported")
except Exception as e:
    print(f"FAIL GameLauncher failed: {e}")

try:
    from capture_screenshot import ScreenshotCapture
    print("OK ScreenshotCapture imported")
except Exception as e:
    print(f"FAIL ScreenshotCapture failed: {e}")

try:
    from test_scenarios import AutomatedTestScenarios, TestStatus
    print("OK test_scenarios imported")
except Exception as e:
    print(f"FAIL test_scenarios failed: {e}")

try:
    from performance_collector import PerformanceCollector, PerformanceMetrics
    print("OK performance_collector imported")
except Exception as e:
    print(f"FAIL performance_collector failed: {e}")

try:
    from issue_reporter import IssueReporter, IssueReport
    print("OK issue_reporter imported")
except Exception as e:
    print(f"FAIL issue_reporter failed: {e}")

try:
    from data_manager import DataManager
    print("OK data_manager imported")
except Exception as e:
    print(f"FAIL data_manager failed: {e}")

try:
    from playtesting_dashboard import PlaytestingDashboard
    print("OK playtesting_dashboard imported")
except Exception as e:
    print(f"FAIL playtesting_dashboard failed: {e}")

print("\nAll imports tested. Checking if logging is still available...")
try:
    logger = logging.getLogger("test")
    logger.info("Logging works!")
    print("OK logging module is working correctly")
except Exception as e:
    print(f"FAIL logging module failed: {e}")