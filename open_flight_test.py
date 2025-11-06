"""
Open the FlightTest level in the editor
"""
import unreal

def open_flight_test():
    print("Opening FlightTest level...")

    # Load the FlightTest level
    level_path = "/Game/FlightTest"
    success = unreal.EditorLevelLibrary.load_level(level_path)

    if success:
        print(f"✓ Successfully loaded {level_path}")

        # Take a screenshot
        unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "flight_test_loaded")
        print("✓ Screenshot taken")
    else:
        print(f"✗ Failed to load {level_path}")

if __name__ == "__main__":
    open_flight_test()
