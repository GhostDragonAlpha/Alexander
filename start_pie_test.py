"""
Start PIE (Play In Editor) and test the automation server
"""
import time
import requests
import sys

def wait_for_editor():
    """Wait for editor to be ready"""
    print("Waiting for Unreal Editor to load...")
    time.sleep(30)  # Give editor time to fully load
    print("Editor should be ready")

def test_automation_server():
    """Test the automation server endpoints"""
    print("\nTesting automation server...")

    # Wait a bit for server to start after PIE begins
    time.sleep(5)

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
        return False
    except requests.exceptions.Timeout:
        print("FAILED: Request timed out")
        return False
    except Exception as e:
        print(f"FAILED: {str(e)}")
        return False

def main():
    # Wait for editor to load
    wait_for_editor()

    print("\n" + "="*60)
    print("INSTRUCTIONS:")
    print("="*60)
    print("1. The Unreal Editor should now be open")
    print("2. Click the PLAY button (or press Alt+P) to start PIE")
    print("3. Watch the Output Log for 'AutomationGameMode: Server started'")
    print("4. The AutomationAPIServer should start on port 8080")
    print("="*60)

    input("\nPress ENTER after you've started PIE...")

    # Test the server
    if test_automation_server():
        print("\n✓ Automation server is working correctly!")
        return 0
    else:
        print("\n✗ Automation server is not responding")
        print("\nCheck the Output Log in Unreal Editor for error messages")
        return 1

if __name__ == "__main__":
    sys.exit(main())
