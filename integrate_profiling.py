"""
Automated Profiling Integration Script
Adds performance profiling to all major systems in the Alexander project
"""

import re
from pathlib import Path
from typing import List, Tuple

class ProfilingIntegrator:
    """Automatically integrates performance profiling into C++ source files"""
    
    def __init__(self, source_dir: str):
        self.source_dir = Path(source_dir)
        self.systems_to_profile = [
            ("Physics/PhysicsConsensusComponent.cpp", "PhysicsConsensus"),
            ("Orbital/OrbitalBody.cpp", "OrbitalMechanics"),
            ("Flight/FlightControllerComponent.cpp", "FlightController"),
            ("Network/NetworkReplicationComponent.cpp", "NetworkReplication"),
        ]
        
    def add_profiler_include(self, content: str) -> str:
        """Add profiler header include if not present"""
        if "PerformanceProfilerSubsystem.h" in content:
            return content
            
        # Find first #include and add after it
        include_pattern = r'(#include\s+"[^"]+\.h")'
        match = re.search(include_pattern, content)
        if match:
            insert_pos = match.end()
            return (content[:insert_pos] + 
                   '\n#include "Profiling/PerformanceProfilerSubsystem.h"' + 
                   content[insert_pos:])
        return content
    
    def add_profiler_member(self, content: str, class_name: str) -> str:
        """Add profiler member variable to class if not present"""
        if "UPerformanceProfiler* Profiler" in content:
            return content
            
        # Find UPROPERTY section or class body
        pattern = rf'class\s+\w+\s+{class_name}\s*{{[^}}]*UPROPERTY\(\)'
        match = re.search(pattern, content, re.DOTALL)
        if match:
            # Add after last UPROPERTY
            uproperty_pattern = r'UPROPERTY\(\)[^;]+;'
            matches = list(re.finditer(uproperty_pattern, content))
            if matches:
                insert_pos = matches[-1].end()
                return (content[:insert_pos] + 
                       '\n\n\tUPROPERTY()\n\tUPerformanceProfiler* Profiler;' +
                       content[insert_pos:])
        return content
    
    def add_profiler_initialization(self, content: str) -> str:
        """Add profiler initialization in InitializeComponent or BeginPlay"""
        if "Profiler = " in content and "GetSubsystem<UPerformanceProfilerSubsystem>" in content:
            return content
            
        # Find InitializeComponent or BeginPlay
        init_pattern = r'void\s+\w+::(?:InitializeComponent|BeginPlay)\(\)[^{]*\{[^}]*Super::\w+\(\);'
        match = re.search(init_pattern, content, re.DOTALL)
        if match:
            insert_pos = match.end()
            profiler_init = """

// Initialize profiler
if (UWorld* World = GetWorld())
{
Profiler = World->GetSubsystem<UPerformanceProfilerSubsystem>();
}"""
            return content[:insert_pos] + profiler_init + content[insert_pos:]
        return content
    
    def add_profiling_to_function(self, content: str, function_name: str, scope_name: str) -> str:
        """Add profiling scope to a specific function"""
        # Check if already profiled
        if f'PROFILE_SCOPE(Profiler, "{scope_name}")' in content:
            return content
            
        # Find function definition
        pattern = rf'void\s+\w+::{function_name}\([^)]*\)[^{{]*\{{'
        match = re.search(pattern, content)
        if match:
            insert_pos = match.end()
            profiling_code = f"""
if (!Profiler) return;
PROFILE_SCOPE(Profiler, "{scope_name}");
"""
            return content[:insert_pos] + profiling_code + content[insert_pos:]
        return content
    
    def integrate_system(self, file_path: Path, system_name: str) -> bool:
        """Integrate profiling into a single system file"""
        if not file_path.exists():
            print(f"[ERROR] File not found: {file_path}")
            return False
            
        print(f"[INFO] Integrating profiling into {file_path.name}...")
        
        try:
            content = file_path.read_text()
            original_content = content
            
            # Add includes
            content = self.add_profiler_include(content)
            
            # Add profiler initialization
            content = self.add_profiler_initialization(content)
            
            # Add profiling to key functions based on system
            if "PhysicsConsensus" in system_name:
                content = self.add_profiling_to_function(
                    content, "ValidatePlayerPositions", "PhysicsConsensus_Validation"
                )
                content = self.add_profiling_to_function(
                    content, "PredictPosition", "PhysicsConsensus_Prediction"
                )
                
            elif "OrbitalMechanics" in system_name:
                content = self.add_profiling_to_function(
                    content, "Tick", "OrbitalMechanics_Tick"
                )
                content = self.add_profiling_to_function(
                    content, "CalculateGravityForces", "OrbitalMechanics_Gravity"
                )
                
            elif "FlightController" in system_name:
                content = self.add_profiling_to_function(
                    content, "ProcessInput", "FlightController_Input"
                )
                content = self.add_profiling_to_function(
                    content, "ApplyThrust", "FlightController_Thrust"
                )
                
            elif "NetworkReplication" in system_name:
                content = self.add_profiling_to_function(
                    content, "ReplicatePosition", "Network_Replication"
                )
            
            # Only write if changed
            if content != original_content:
                # Create backup
                backup_path = file_path.with_suffix(file_path.suffix + ".backup")
                backup_path.write_text(original_content)
                
                # Write modified content
                file_path.write_text(content)
                print(f"[OK] Profiling integrated (backup: {backup_path.name})")
                return True
            else:
                print(f"[INFO] Already has profiling")
                return False
                
        except Exception as e:
            print(f"[ERROR] {e}")
            return False
    
    def integrate_all(self) -> tuple:
        """Integrate profiling into all target systems"""
        success_count = 0
        total_count = len(self.systems_to_profile)
        
        print(f"\n[INFO] Starting profiling integration for {total_count} systems...\n")
        
        for relative_path, system_name in self.systems_to_profile:
            file_path = self.source_dir / relative_path
            if self.integrate_system(file_path, system_name):
                success_count += 1
            print()
        
        print(f"\n[SUMMARY] Integration Results:")
        print(f"[OK] Successful: {success_count}/{total_count}")
        print(f"[INFO] Skipped: {total_count - success_count}/{total_count}")
        
        return success_count, total_count
    
    def create_profiling_test(self) -> None:
        """Create a test map/level for profiling"""
        test_content = """# Profiling Test Instructions

## Setup
1. Open Unreal Editor
2. Load any test level
3. Enable profiling with console command: `Profile.Start`

## Run Tests
1. Spawn multiple ships (10+)
2. Enable physics simulation
3. Fly around for 30 seconds
4. Run `Profile.Log` to see current stats

## Stop and Export
1. Run `Profile.Stop` to stop profiling
2. Report saved to: `Saved/Profiling/manual_profile.json`

## Performance Targets
- Physics Consensus:  < 5ms
- Orbital Mechanics:  < 3ms
- Flight Controller:  < 2ms
- Network:           < 1ms
- Total:             < 11ms (90fps)

## Analyze Results
Check JSON report for:
- Average frame time
- Peak memory usage
- Per-system bottlenecks
- Sample counts
"""
        
        test_file = self.source_dir.parent / "PROFILING_TEST_GUIDE.md"
        test_file.write_text(test_content.strip())
        print(f"[INFO] Created profiling test guide: {test_file}")

def main():
    """Main integration script"""
    import sys
    
    if len(sys.argv) > 1:
        source_dir = sys.argv[1]
    else:
        source_dir = "Source/Alexander"
    
    integrator = ProfilingIntegrator(source_dir)
    
    print("=" * 60)
    print("  Alexander Performance Profiling Integration")
    print("=" * 60)
    
    # Integrate profiling
    success, total = integrator.integrate_all()
    
    # Create test guide
    integrator.create_profiling_test()
    
    print("\n" + "=" * 60)
    print("  Next Steps:")
    print("=" * 60)
    print("1. Build the project: python build_and_test.py build")
    print("2. Open in UE Editor")
    print("3. Run console command: Profile.Start")
    print("4. Test gameplay for 1-2 minutes")
    print("5. Run console command: Profile.Log")
    print("6. Run console command: Profile.Stop")
    print("7. Check report: Saved/Profiling/manual_profile.json")
    print("\n[OK] Profiling integration complete!\n")

if __name__ == "__main__":
    main()
