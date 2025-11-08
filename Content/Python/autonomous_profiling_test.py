"""
Unreal Editor Python Script - Execute Profiling Test
Run this directly in the Unreal Editor Python console or via File > Execute Python Script
"""

import unreal
import time

def run_profiling_test():
    """Execute autonomous profiling test within Unreal Editor"""
    
    unreal.log("\n" + "="*70)
    unreal.log("🚀 AUTONOMOUS PROFILING TEST STARTING")
    unreal.log("="*70)
    
    # Get the world
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        unreal.log_error("❌ No valid world found!")
        return False
    
    unreal.log(f"✅ World: {world.get_name()}")
    
    # Get profiler subsystem
    try:
        profiler = world.get_subsystem(unreal.PerformanceProfilerSubsystem)
        if not profiler:
            unreal.log_error("❌ PerformanceProfilerSubsystem not found!")
            unreal.log_error("   Make sure the C++ class is compiled and registered")
            return False
        
        unreal.log("✅ PerformanceProfilerSubsystem acquired")
    except Exception as e:
        unreal.log_error(f"❌ Failed to get profiler: {e}")
        return False
    
    # Start profiling
    try:
        profiler.start_profiling()
        unreal.log("✅ Profiling STARTED")
    except Exception as e:
        unreal.log_error(f"❌ Failed to start profiling: {e}")
        return False
    
    # Run test for 2 minutes
    unreal.log("\n⏱ Running 2-minute performance test...")
    unreal.log("   Testing current editor state with all active systems")
    
    test_duration = 120  # seconds
    start_time = time.time()
    last_log = start_time
    
    while time.time() - start_time < test_duration:
        elapsed = time.time() - start_time
        
        # Log progress every 15 seconds
        if time.time() - last_log >= 15:
            remaining = test_duration - elapsed
            unreal.log(f"⏳ Progress: {int(elapsed)}/{test_duration}s ({int(remaining)}s remaining)")
            last_log = time.time()
        
        # Small sleep to not block editor
        time.sleep(0.1)
    
    unreal.log("\n✅ Test duration complete")
    
    # Get results before stopping
    try:
        unreal.log("\n" + "="*70)
        unreal.log("📊 PROFILING RESULTS")
        unreal.log("="*70)
        
        # Memory stats
        current_mem = profiler.get_current_memory_usage_mb()
        peak_mem = profiler.get_peak_memory_usage_mb()
        
        unreal.log(f"\n💾 MEMORY USAGE:")
        unreal.log(f"   Current: {current_mem:.1f} MB")
        unreal.log(f"   Peak:    {peak_mem:.1f} MB")
        
        if peak_mem > 8192:
            unreal.log_warning(f"   ⚠️  EXCEEDS TARGET (8192 MB)")
        else:
            unreal.log(f"   ✅ Within target (8192 MB)")
        
        # Top bottlenecks
        unreal.log(f"\n🔥 TOP PERFORMANCE BOTTLENECKS:")
        bottlenecks = profiler.get_top_bottlenecks(10)
        
        if not bottlenecks or len(bottlenecks) == 0:
            unreal.log("   No bottleneck data collected")
            unreal.log("   This may indicate no systems were actively profiled")
        else:
            for i, bottleneck in enumerate(bottlenecks, 1):
                avg_time = bottleneck.average_time_ms
                max_time = bottleneck.max_time_ms
                system_name = bottleneck.system_name
                sample_count = bottleneck.sample_count
                
                # Determine status
                if avg_time > 8.0:
                    status = "❌ CRITICAL"
                elif avg_time > 5.0:
                    status = "⚠️  WARNING"
                else:
                    status = "✅ OK"
                
                unreal.log(f"\n   {i}. {system_name}")
                unreal.log(f"      Avg: {avg_time:.2f}ms | Max: {max_time:.2f}ms | Samples: {sample_count}")
                unreal.log(f"      {status}")
        
    except Exception as e:
        unreal.log_error(f"❌ Error getting results: {e}")
    
    # Stop profiling (auto-exports JSON)
    try:
        profiler.stop_profiling()
        unreal.log("\n✅ Profiling STOPPED")
        unreal.log("   Results exported to: Saved/Profiling/profile_[timestamp].json")
    except Exception as e:
        unreal.log_error(f"❌ Failed to stop profiling: {e}")
        return False
    
    unreal.log("\n" + "="*70)
    unreal.log("✅ AUTONOMOUS PROFILING TEST COMPLETE")
    unreal.log("="*70)
    unreal.log("\n📄 Check these files:")
    unreal.log("   • Saved/Profiling/profile_*.json (detailed metrics)")
    unreal.log("   • Output Log (this console) for summary\n")
    
    return True

# Execute the test
if __name__ == '__main__':
    success = run_profiling_test()
    if success:
        unreal.log("✅ Test completed successfully!")
    else:
        unreal.log_error("❌ Test failed - check errors above")
