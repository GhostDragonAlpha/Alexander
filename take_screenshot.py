import mss
import mss.tools

with mss.mss() as sct:
    # Take screenshot of primary monitor
    screenshot = sct.shot(output='C:/Users/allen/Desktop/Alexander/Alexander/current_screen.png')
    print(f"Screenshot saved: {screenshot}")
