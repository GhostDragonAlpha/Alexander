"""Wait for server to be ready"""
import requests
import time
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
url = f"http://localhost:{port}/status"

print(f"\nWaiting for server on port {port}...")

for attempt in range(60):  # Try for 60 seconds
    try:
        response = requests.get(url, timeout=2)
        if response.status_code == 200:
            print(f"[READY] Server on port {port} is ready!")
            print(response.json())
            sys.exit(0)
    except:
        pass

    print(f"Attempt {attempt + 1}/60...", end="\r")
    time.sleep(1)

print(f"\n[TIMEOUT] Server on port {port} did not start within 60 seconds")
sys.exit(1)
