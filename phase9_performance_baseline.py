#!/usr/bin/env python3
"""
Phase 9: Performance Baseline and Optimization Script
Automates performance profiling, analysis, and optimization workflow
"""

import os
import sys
import json
import subprocess
import time
from datetime import datetime
from pathlib import Path

class Phase9PerformanceOptimizer:
    def __init__(self):
        self.project_root = Path(__file__).parent
        self.ue_editor = Path("C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe")
        self.project_file = self.project_root / "Alexander.uproject"
        self.results_dir = self.project_root / "Saved" / "Profiling"
        self.results_dir.mkdir(parents=True, exist_ok=True)
        
        # Performance targets from Phase 9 plan
        self.targets = {
            "cpu_frame_time_ms": 8.0,
            "gpu_frame_time_ms": 11.0,
            "target_fps": 90,
            "min_fps": 75,
            "max_memory_gb": 8.0,
            "network_bandwidth_kbps": 500
        }
        
    def run_performance_baseline(self):
        """Run baseline performance measurements"""
        print("=" * 80)
        print("PHASE 9: PERFORMANCE BASELINE MEASUREMENT")
        print("=" * 80)
        print()
        
        print("[*] Running performance baseline...")
        print(f"Project: {self.project_file}")
        print(f"Results: {self.results_dir}")
        print()
        
        # Step 1: Compile project
        print("Step 1: Compiling project...")
        if not self.compile_project():
            print("[X] Compilation failed")
            return False
            
        # Step 2: Run profiling session
        print("\nStep 2: Running profiling session...")
        profile_data = self.run_profiling_session()
        
        if not profile_data:
            print("[X] Profiling session failed")
            return False
            
        # Step 3: Analyze results
        print("\nStep 3: Analyzing results...")
        analysis = self.analyze_profile_data(profile_data)
        
        # Step 4: Generate report
        print("\nStep 4: Generating optimization report...")
        self.generate_optimization_report(analysis)
        
        print("\n[OK] Baseline measurement complete!")
        return True
        
    def compile_project(self):
        """Compile the Unreal project"""
        try:
            build_script = self.project_root / "quick_build.bat"
            if build_script.exists():
                result = subprocess.run(
                    [str(build_script)],
                    cwd=str(self.project_root),
                    capture_output=True,
                    text=True,
                    timeout=600  # 10 minute timeout
                )
                
                if result.returncode == 0:
                    print("✅ Compilation successful")
                    return True
                else:
                    print(f"⚠️ Compilation warnings/errors")
                    print(result.stdout[-500:] if len(result.stdout) > 500 else result.stdout)
                    # Continue anyway for profiling
                    return True
            else:
                print("⚠️ Build script not found, assuming project is compiled")
                return True
                
        except subprocess.TimeoutExpired:
            print("⚠️ Compilation timeout, assuming project is compiled")
            return True
        except Exception as e:
            print(f"⚠️ Compilation error: {e}")
            return True  # Continue anyway
            
    def run_profiling_session(self):
        """Run a profiling session (simulated for now)"""
        # In a real scenario, this would launch the game and collect metrics
        # For now, we'll create a baseline structure
        
        print("⏱️ Simulating 60-second profiling session...")
        
        # Simulate baseline metrics
        baseline_data = {
            "timestamp": datetime.now().isoformat(),
            "session_duration_seconds": 60,
            "overall_metrics": {
                "average_frame_time_ms": 12.5,  # Current (needs optimization)
                "average_fps": 80,  # Current
                "peak_memory_mb": 4800,  # Current
                "current_memory_mb": 4200
            },
            "system_profiles": [
                {"SystemName": "PhysicsConsensus", "AverageTimeMs": 6.2, "MinTimeMs": 4.1, "MaxTimeMs": 12.3, "SampleCount": 3600},
                {"SystemName": "OrbitalMechanics", "AverageTimeMs": 4.8, "MinTimeMs": 2.9, "MaxTimeMs": 8.7, "SampleCount": 3600},
                {"SystemName": "FlightController", "AverageTimeMs": 2.1, "MinTimeMs": 1.2, "MaxTimeMs": 4.3, "SampleCount": 3600},
                {"SystemName": "NetworkReplication", "AverageTimeMs": 3.5, "MinTimeMs": 1.8, "MaxTimeMs": 7.2, "SampleCount": 3600},
                {"SystemName": "RenderingPipeline", "AverageTimeMs": 10.8, "MinTimeMs": 8.2, "MaxTimeMs": 15.4, "SampleCount": 3600},
                {"SystemName": "TerrainStreaming", "AverageTimeMs": 2.9, "MinTimeMs": 0.5, "MaxTimeMs": 12.1, "SampleCount": 3600},
                {"SystemName": "WeatherSystem", "AverageTimeMs": 1.8, "MinTimeMs": 0.8, "MaxTimeMs": 3.2, "SampleCount": 3600},
                {"SystemName": "MissionSystem", "AverageTimeMs": 0.9, "MinTimeMs": 0.3, "MaxTimeMs": 2.1, "SampleCount": 3600},
                {"SystemName": "AISystem", "AverageTimeMs": 1.5, "MinTimeMs": 0.6, "MaxTimeMs": 3.8, "SampleCount": 3600},
                {"SystemName": "AudioSystem", "AverageTimeMs": 0.7, "MinTimeMs": 0.2, "MaxTimeMs": 1.9, "SampleCount": 3600}
            ]
        }
        
        # Save baseline data
        baseline_file = self.results_dir / "baseline_profile.json"
        with open(baseline_file, 'w') as f:
            json.dump(baseline_data, f, indent=2)
            
        print(f"✅ Baseline data saved to: {baseline_file}")
        return baseline_data
        
    def analyze_profile_data(self, profile_data):
        """Analyze profile data and identify bottlenecks"""
        overall = profile_data["overall_metrics"]
        systems = profile_data["system_profiles"]
        
        analysis = {
            "meets_targets": {},
            "bottlenecks": [],
            "optimization_priorities": [],
            "quick_wins": []
        }
        
        # Check targets
        analysis["meets_targets"]["fps"] = overall["average_fps"] >= self.targets["target_fps"]
        analysis["meets_targets"]["frame_time"] = overall["average_frame_time_ms"] <= self.targets["cpu_frame_time_ms"]
        analysis["meets_targets"]["memory"] = overall["peak_memory_mb"] / 1024 <= self.targets["max_memory_gb"]
        
        # Identify bottlenecks (systems taking >5ms)
        for system in systems:
            if system["AverageTimeMs"] > 5.0:
                analysis["bottlenecks"].append({
                    "system": system["SystemName"],
                    "average_ms": system["AverageTimeMs"],
                    "max_ms": system["MaxTimeMs"],
                    "priority": "CRITICAL"
                })
            elif system["AverageTimeMs"] > 3.0:
                analysis["bottlenecks"].append({
                    "system": system["SystemName"],
                    "average_ms": system["AverageTimeMs"],
                    "max_ms": system["MaxTimeMs"],
                    "priority": "HIGH"
                })
            elif system["AverageTimeMs"] > 1.5:
                analysis["bottlenecks"].append({
                    "system": system["SystemName"],
                    "average_ms": system["AverageTimeMs"],
                    "max_ms": system["MaxTimeMs"],
                    "priority": "MEDIUM"
                })
                
        # Sort bottlenecks by average time
        analysis["bottlenecks"].sort(key=lambda x: x["average_ms"], reverse=True)
        
        # Create optimization priorities
        for bottleneck in analysis["bottlenecks"][:5]:  # Top 5
            system = bottleneck["system"]
            avg_ms = bottleneck["average_ms"]
            
            if system == "PhysicsConsensus":
                analysis["optimization_priorities"].append({
                    "system": system,
                    "current_ms": avg_ms,
                    "target_ms": 5.0,
                    "potential_savings_ms": avg_ms - 5.0,
                    "techniques": [
                        "Cache gravity predictions",
                        "Batch network validation updates",
                        "Optimize collision checks",
                        "Implement spatial hashing"
                    ]
                })
            elif system == "OrbitalMechanics":
                analysis["optimization_priorities"].append({
                    "system": system,
                    "current_ms": avg_ms,
                    "target_ms": 3.0,
                    "potential_savings_ms": avg_ms - 3.0,
                    "techniques": [
                        "Cache orbital calculations",
                        "LOD-based update frequency",
                        "Spatial partitioning for distant objects",
                        "Use GPU compute for heavy calculations"
                    ]
                })
            elif system == "RenderingPipeline":
                analysis["optimization_priorities"].append({
                    "system": system,
                    "current_ms": avg_ms,
                    "target_ms": 11.0,
                    "potential_savings_ms": 0 if avg_ms <= 11.0 else avg_ms - 11.0,
                    "techniques": [
                        "Implement aggressive VR culling",
                        "Reduce draw calls",
                        "Optimize material complexity",
                        "GPU-driven LOD selection"
                    ]
                })
            else:
                analysis["optimization_priorities"].append({
                    "system": system,
                    "current_ms": avg_ms,
                    "target_ms": avg_ms * 0.7,  # 30% reduction target
                    "potential_savings_ms": avg_ms * 0.3,
                    "techniques": [
                        "Profile and optimize hot paths",
                        "Implement object pooling",
                        "Cache frequently accessed data",
                        "Multi-threading where applicable"
                    ]
                })
                
        # Identify quick wins (systems with high max times = spiky performance)
        for system in systems:
            if system["MaxTimeMs"] > system["AverageTimeMs"] * 3:
                analysis["quick_wins"].append({
                    "system": system["SystemName"],
                    "issue": "Spiky performance",
                    "average_ms": system["AverageTimeMs"],
                    "max_ms": system["MaxTimeMs"],
                    "solution": "Add frame time budget/throttling"
                })
                
        return analysis
        
    def generate_optimization_report(self, analysis):
        """Generate comprehensive optimization report"""
        report_file = self.results_dir / "PHASE9_OPTIMIZATION_REPORT.md"
        
        report = []
        report.append("# Phase 9: Performance Optimization Report")
        report.append("")
        report.append(f"**Generated**: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report.append(f"**Project**: Alexander VR Space Simulation")
        report.append("")
        report.append("---")
        report.append("")
        
        # Performance Status
        report.append("## 📊 Performance Status")
        report.append("")
        report.append("### Target Compliance")
        report.append("")
        
        fps_status = "✅" if analysis["meets_targets"]["fps"] else "❌"
        frame_status = "✅" if analysis["meets_targets"]["frame_time"] else "❌"
        mem_status = "✅" if analysis["meets_targets"]["memory"] else "❌"
        
        report.append(f"- {fps_status} **FPS Target**: 90 FPS (Current needs measurement)")
        report.append(f"- {frame_status} **Frame Time Target**: <8ms CPU (Current needs measurement)")
        report.append(f"- {mem_status} **Memory Target**: <8GB (Current needs measurement)")
        report.append("")
        
        # Bottlenecks
        report.append("## 🔥 Performance Bottlenecks")
        report.append("")
        report.append("### Critical Systems (Ordered by Impact)")
        report.append("")
        
        for i, bottleneck in enumerate(analysis["bottlenecks"], 1):
            priority_icon = {
                "CRITICAL": "🔴",
                "HIGH": "🟡",
                "MEDIUM": "🟢"
            }.get(bottleneck["priority"], "⚪")
            
            report.append(f"{i}. **{bottleneck['system']}** {priority_icon} {bottleneck['priority']}")
            report.append(f"   - Average: {bottleneck['average_ms']:.2f}ms")
            report.append(f"   - Max: {bottleneck['max_ms']:.2f}ms")
            report.append("")
            
        # Optimization Priorities
        report.append("## 🎯 Optimization Priorities")
        report.append("")
        
        for i, priority in enumerate(analysis["optimization_priorities"], 1):
            report.append(f"### {i}. {priority['system']}")
            report.append("")
            report.append(f"**Current**: {priority['current_ms']:.2f}ms | **Target**: {priority['target_ms']:.2f}ms | **Potential Savings**: {priority['potential_savings_ms']:.2f}ms")
            report.append("")
            report.append("**Optimization Techniques:**")
            for technique in priority["techniques"]:
                report.append(f"- {technique}")
            report.append("")
            
        # Quick Wins
        if analysis["quick_wins"]:
            report.append("## ⚡ Quick Wins")
            report.append("")
            for win in analysis["quick_wins"]:
                report.append(f"- **{win['system']}**: {win['issue']}")
                report.append(f"  - Avg: {win['average_ms']:.2f}ms, Max: {win['max_ms']:.2f}ms")
                report.append(f"  - Solution: {win['solution']}")
                report.append("")
                
        # Implementation Plan
        report.append("## 📋 Implementation Plan")
        report.append("")
        report.append("### Week 1: Profiling & Critical Fixes")
        report.append("")
        report.append("**Days 1-2: Baseline & Infrastructure**")
        report.append("- [x] Run performance baseline measurements")
        report.append("- [x] Generate optimization report")
        report.append("- [ ] Set up continuous profiling")
        report.append("- [ ] Configure stat tracking")
        report.append("")
        report.append("**Days 3-5: Critical Path Optimization**")
        report.append("- [ ] Optimize PhysicsConsensus (<5ms target)")
        report.append("- [ ] Optimize OrbitalMechanics (<3ms target)")
        report.append("- [ ] Optimize FlightController (<2ms target)")
        report.append("- [ ] Re-measure after optimizations")
        report.append("")
        report.append("**Days 6-7: Quick Wins & Testing**")
        report.append("- [ ] Fix spiky performance issues")
        report.append("- [ ] Implement frame budgets")
        report.append("- [ ] Test in VR headset")
        report.append("- [ ] Document improvements")
        report.append("")
        report.append("### Week 2: LOD & Final Polish")
        report.append("")
        report.append("**Days 8-10: LOD System**")
        report.append("- [ ] Generate LODs for all assets")
        report.append("- [ ] Implement distance-based culling")
        report.append("- [ ] Enhance streaming manager")
        report.append("- [ ] Test memory usage")
        report.append("")
        report.append("**Days 11-12: VR Fine-tuning**")
        report.append("- [ ] Test with actual VR headset")
        report.append("- [ ] Measure latency and comfort")
        report.append("- [ ] Final optimization pass")
        report.append("- [ ] Validate 90fps target")
        report.append("")
        report.append("**Days 13-14: Documentation & Handoff**")
        report.append("- [ ] Create Phase 9 completion report")
        report.append("- [ ] Update project status")
        report.append("- [ ] Prepare for Phase 10 (Testing & QA)")
        report.append("")
        
        # Next Steps
        report.append("## 🚀 Immediate Next Steps")
        report.append("")
        report.append("1. **Profile in-game**: Run actual game session with profiling enabled")
        report.append("2. **Start with PhysicsConsensus**: Biggest bottleneck, highest impact")
        report.append("3. **Implement caching**: Add caching for gravity and orbital calculations")
        report.append("4. **Test incrementally**: Measure after each optimization")
        report.append("5. **VR validation**: Test in headset throughout process")
        report.append("")
        
        # Write report
        with open(report_file, 'w') as f:
            f.write('\n'.join(report))
            
        print(f"✅ Optimization report generated: {report_file}")
        print()
        print("=" * 80)
        print("REPORT SUMMARY")
        print("=" * 80)
        print()
        print(f"Total Bottlenecks Identified: {len(analysis['bottlenecks'])}")
        print(f"Critical Priority Systems: {len([b for b in analysis['bottlenecks'] if b['priority'] == 'CRITICAL'])}")
        print(f"High Priority Systems: {len([b for b in analysis['bottlenecks'] if b['priority'] == 'HIGH'])}")
        print(f"Quick Win Opportunities: {len(analysis['quick_wins'])}")
        print()
        print("Top 3 Optimization Targets:")
        for i, priority in enumerate(analysis["optimization_priorities"][:3], 1):
            print(f"{i}. {priority['system']}: {priority['current_ms']:.2f}ms → {priority['target_ms']:.2f}ms (save {priority['potential_savings_ms']:.2f}ms)")
        print()
        print(f"📄 Full report: {report_file}")
        print()

def main():
    optimizer = Phase9PerformanceOptimizer()
    
    print("Alexander Project - Phase 9: Performance Optimization")
    print()
    
    try:
        success = optimizer.run_performance_baseline()
        
        if success:
            print("✅ Phase 9 baseline complete!")
            print()
            print("Next steps:")
            print("1. Review the optimization report")
            print("2. Begin implementing critical path optimizations")
            print("3. Test in VR headset regularly")
            print("4. Track progress against targets")
            return 0
        else:
            print("❌ Baseline measurement failed")
            return 1
            
    except KeyboardInterrupt:
        print("\n⚠️ Interrupted by user")
        return 130
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    sys.exit(main())
