#!/usr/bin/env python3
"""
Maximize and focus the Unreal Editor window
"""
import win32gui
import win32con
import win32api
import time

def find_unreal_window():
    """Find the Unreal Editor window"""
    windows = []

    def callback(hwnd, windows):
        if win32gui.IsWindowVisible(hwnd):
            title = win32gui.GetWindowText(hwnd)
            # Must contain "Unreal Editor" specifically
            if "Unreal Editor" in title:
                windows.append((hwnd, title))

    win32gui.EnumWindows(callback, windows)

    # Sort by preference - main editor window first
    windows.sort(key=lambda x: ("Unreal Editor" in x[1], "Alexander" in x[1]), reverse=True)

    return windows

def maximize_unreal():
    """Maximize and focus Unreal Editor"""
    windows = find_unreal_window()

    if not windows:
        print("[!] No Unreal Editor window found")
        return False

    # Use the first matching window
    hwnd, title = windows[0]
    print(f"[+] Found window: {title}")

    # Restore if minimized
    if win32gui.IsIconic(hwnd):
        win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)
        time.sleep(0.5)

    # Force maximize - multiple attempts
    win32gui.ShowWindow(hwnd, win32con.SW_MAXIMIZE)
    time.sleep(0.3)

    # Set window to topmost temporarily to force it to front
    win32gui.SetWindowPos(hwnd, win32con.HWND_TOPMOST, 0, 0, 0, 0,
                         win32con.SWP_NOMOVE | win32con.SWP_NOSIZE)
    time.sleep(0.2)

    # Remove topmost flag but keep it in front
    win32gui.SetWindowPos(hwnd, win32con.HWND_NOTOPMOST, 0, 0, 0, 0,
                         win32con.SWP_NOMOVE | win32con.SWP_NOSIZE | win32con.SWP_SHOWWINDOW)
    time.sleep(0.2)

    # One more maximize to ensure it's fully maximized
    win32gui.ShowWindow(hwnd, win32con.SW_MAXIMIZE)
    time.sleep(0.3)

    # Verify it's maximized
    placement = win32gui.GetWindowPlacement(hwnd)
    if placement[1] == win32con.SW_SHOWMAXIMIZED:
        print("[+] Window successfully maximized")
    else:
        print("[!] Window may not be fully maximized")
        # Try one more time
        win32gui.ShowWindow(hwnd, win32con.SW_MAXIMIZE)
        time.sleep(0.3)

    return True

if __name__ == "__main__":
    maximize_unreal()
