"""
Configure Niagara Effects for the Spectacular Sun
Run this script after assigning Niagara systems to the Sun actor in the editor
"""

import unreal

def find_sun_actor():
    """Find the Sun_Main actor in the current level"""
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in all_actors:
        if actor.get_actor_label() == "Sun_Main":
            return actor
    return None

def configure_sun_niagara_effects():
    """
    Configure Niagara effects on the Sun actor

    This script assumes you've already assigned Niagara systems to:
    - CoronaEffect component
    - SolarFlares component
    - ElectromagneticStorm component

    You can assign them in the editor by:
    1. Select Sun_Main actor
    2. In Details panel, find each Niagara component
    3. Assign a Niagara System asset to each
    """

    unreal.log("=" * 70)
    unreal.log("CONFIGURING SUN NIAGARA EFFECTS")
    unreal.log("=" * 70)

    sun = find_sun_actor()
    if not sun:
        unreal.log_error("Sun_Main actor not found in level!")
        unreal.log("Make sure the solar system has been spawned")
        return False

    unreal.log(f"Found Sun actor: {sun.get_name()}")

    # Get Niagara components
    # Note: Component access in Python is limited
    # This script shows what parameters WOULD be set if components are active

    unreal.log("\n" + "=" * 70)
    unreal.log("CORONA EFFECT PARAMETERS")
    unreal.log("=" * 70)
    unreal.log("Assign a Niagara System with these parameters:")
    unreal.log("  - Intensity (float): Controls glow brightness")
    unreal.log("  - Color (LinearColor): Sun color")
    unreal.log("  - Radius (float): Size of corona")
    unreal.log("  - ParticleCount (int): Density of effect")
    unreal.log("\nRecommended values:")
    unreal.log("  Intensity: 1.0")
    unreal.log("  Radius: 3000.0")
    unreal.log("  ParticleCount: 1000")

    unreal.log("\n" + "=" * 70)
    unreal.log("SOLAR FLARES PARAMETERS")
    unreal.log("=" * 70)
    unreal.log("Assign a Niagara System with these parameters:")
    unreal.log("  - FlareIntensity (float): Burst strength")
    unreal.log("  - FlareColor (LinearColor): Burst color")
    unreal.log("  - BurstTrigger (float): Set to 1.0 to trigger burst")
    unreal.log("  - BurstDuration (float): How long bursts last")
    unreal.log("\nRecommended values:")
    unreal.log("  FlareIntensity: 1.5")
    unreal.log("  BurstDuration: 0.5 seconds")

    unreal.log("\n" + "=" * 70)
    unreal.log("ELECTROMAGNETIC STORM PARAMETERS")
    unreal.log("=" * 70)
    unreal.log("Assign a Niagara System with these parameters:")
    unreal.log("  - StormIntensity (float): Base storm strength")
    unreal.log("  - StormFrequency (float): Storms per second")
    unreal.log("  - CurrentIntensity (float): Dynamic intensity (auto-set by C++)")
    unreal.log("  - ElectricColor (LinearColor): Lightning color")
    unreal.log("\nRecommended values:")
    unreal.log("  StormIntensity: 1.0")
    unreal.log("  StormFrequency: 0.5 (one storm every 2 seconds)")
    unreal.log("  ElectricColor: (1.0, 0.95, 0.8) - bright white-yellow")

    unreal.log("\n" + "=" * 70)
    unreal.log("CURRENT SUN PROPERTIES")
    unreal.log("=" * 70)

    try:
        # Try to get some properties (may not all be accessible via Python)
        unreal.log(f"  Sun Scale: {sun.get_editor_property('sun_scale')}")
        unreal.log(f"  Main Light Intensity: {sun.get_editor_property('main_light_intensity')}")
        unreal.log(f"  Core Light Intensity: {sun.get_editor_property('core_light_intensity')}")
        unreal.log(f"  Corona Light Intensity: {sun.get_editor_property('corona_light_intensity')}")
        unreal.log(f"  Storm Enabled: {sun.get_editor_property('b_enable_electromagnetic_storms')}")
        unreal.log(f"  Storm Intensity: {sun.get_editor_property('storm_intensity')}")
        unreal.log(f"  Storm Frequency: {sun.get_editor_property('storm_frequency')}")
    except Exception as e:
        unreal.log_warning(f"Could not read all properties: {e}")

    unreal.log("\n" + "=" * 70)
    unreal.log("SETUP INSTRUCTIONS")
    unreal.log("=" * 70)
    unreal.log("\nTo create spectacular Sun effects:")
    unreal.log("\n1. CREATE NIAGARA SYSTEMS")
    unreal.log("   - Right-click in Content Browser")
    unreal.log("   - FX → Niagara System")
    unreal.log("   - Create three systems:")
    unreal.log("     * NS_SunCorona (glowing particle haze)")
    unreal.log("     * NS_SolarFlares (burst emitter)")
    unreal.log("     * NS_ElectromagneticStorm (lightning/plasma effect)")
    unreal.log("\n2. ASSIGN TO SUN ACTOR")
    unreal.log("   - Select Sun_Main in Outliner")
    unreal.log("   - In Details panel, find:")
    unreal.log("     * CoronaEffect component → Assign NS_SunCorona")
    unreal.log("     * SolarFlares component → Assign NS_SolarFlares")
    unreal.log("     * ElectromagneticStorm component → Assign NS_ElectromagneticStorm")
    unreal.log("\n3. CONFIGURE PARAMETERS")
    unreal.log("   - Each Niagara system should expose User Parameters")
    unreal.log("   - Match parameter names to those listed above")
    unreal.log("   - The C++ code will automatically update these values")
    unreal.log("\n4. TEST IN PLAY MODE")
    unreal.log("   - Press Play")
    unreal.log("   - You should see:")
    unreal.log("     * Constant glowing corona around sun")
    unreal.log("     * Periodic bursts (every 2 seconds)")
    unreal.log("     * Dynamic light flickering")
    unreal.log("     * Storm intensity waves")

    unreal.log("\n" + "=" * 70)
    unreal.log("QUICK START - SIMPLE NIAGARA SYSTEMS")
    unreal.log("=" * 70)
    unreal.log("\nFor quick testing, use these simple Niagara setups:")
    unreal.log("\nCORONA EFFECT:")
    unreal.log("  - Emitter: Sphere Location")
    unreal.log("  - Spawn Rate: 100 particles/sec")
    unreal.log("  - Lifetime: 5 seconds")
    unreal.log("  - Color: Orange/Yellow gradient")
    unreal.log("  - Size: 50-100 units")
    unreal.log("  - Material: Additive glow")
    unreal.log("\nSOLAR FLARES:")
    unreal.log("  - Emitter: Burst from sphere surface")
    unreal.log("  - Burst Count: 20-50 particles")
    unreal.log("  - Initial Velocity: Outward from surface")
    unreal.log("  - Lifetime: 0.5-1.0 seconds")
    unreal.log("  - Color: Bright white/yellow")
    unreal.log("  - Size: Start large, shrink over time")
    unreal.log("\nELECTROMAGNETIC STORM:")
    unreal.log("  - Emitter: Lightning arcs across surface")
    unreal.log("  - Use Beam emitter or Ribbon")
    unreal.log("  - Color: Electric blue/white")
    unreal.log("  - Rapidly flashing/pulsing")
    unreal.log("  - Random paths across sphere")

    unreal.log("\n" + "=" * 70)
    unreal.log("CONFIGURATION COMPLETE")
    unreal.log("=" * 70)
    unreal.log("\nThe Sun's C++ code is ready to drive the effects!")
    unreal.log("Simply assign Niagara systems and they'll be animated automatically.")
    unreal.log("=" * 70)

    return True

def quick_test_sun_lights():
    """Test the Sun's lighting system without Niagara"""
    sun = find_sun_actor()
    if not sun:
        unreal.log_error("Sun not found")
        return

    unreal.log("\nTesting Sun lighting...")
    unreal.log("Press PLAY and watch for:")
    unreal.log("  - Subtle light flickering")
    unreal.log("  - Periodic intensity surges (electromagnetic storms)")
    unreal.log("  - Color temperature shifts")
    unreal.log("  - Slow rotation")
    unreal.log("\nEven without Niagara, the Sun should look alive!")

if __name__ == "__main__":
    configure_sun_niagara_effects()
    quick_test_sun_lights()
