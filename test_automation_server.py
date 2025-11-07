"""
Test the automation server - automated version
"""
import time
import requests
import sys

def test_server():
    """Test the automation server"""
    print("Testing automation server on port 8080...")

    try:
        response = requests.get('http://localhost:8080/status', timeout=5)
        if response.status_code == 200:
            print(f"SUCCESS: Server responded with status {response.status_code}")
            print(f"Response: {response.text}")
            return True
        else:
            print(f"FAILED: Server returned status {response.status_code}")
            print(f"Response: {response.text}")
            return False
    except requests.exceptions.ConnectionError:
        print("FAILED: Could not connect to server on port 8080")
        print("The server is not running or not listening on port 8080")
        return False
    except requests.exceptions.Timeout:
        print("FAILED: Request timed out")
        return False
    except Exception as e:
        print(f"FAILED: {str(e)}")
        return False

if __name__ == "__main__":
    sys.exit(0 if test_server() else 1)
