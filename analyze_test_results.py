#!/usr/bin/env python3
"""
Test Results Analyzer for Alexander
Analyzes verification reports and provides actionable feedback
"""

import json
import sys
from pathlib import Path

class TestResultsAnalyzer:
    def __init__(self, report_path):
        self.report_path = Path(report_path)
        self.results = self.load_results()
    
    def load_results(self):
        """Load test results from JSON file"""
        with open(self.report_path, 'r') as f:
            return json.load(f)
    
    def analyze(self):
        """Comprehensive analysis of test results"""
        print("=== Test Results Analysis ===")
        
        # Overall result
        if self.results["overall_success"]:
            print("✅ OVERALL: All tests PASSED")
        else:
            print("❌ OVERALL: Some tests FAILED")
        
        # Test summary
        print(f"\nTest Summary:")
        print(f"  Total: {self.results['tests_run']}")
        print(f"  Passed: {self.results['tests_passed']}")
        print(f"  Failed: {self.results['tests_failed']}")
        
        # System-by-system results
        if self.results["system_results"]:
            print(f"\nSystem Results:")
            for system in self.results["system_results"]:
                status = "✅ PASSED" if system["passed"] else "❌ FAILED"
                print(f"  {system['system']}: {status}")
                print(f"    Duration: {system['duration_seconds']}s")
                print(f"    FPS: {system['avg_fps']}")
        
        # Performance analysis
        perf = self.results["performance"]
        print(f"\nPerformance Analysis:")
        print(f"  FPS Min: {perf['fps_min']:.1f}")
        print(f"  FPS Max: {perf['fps_max']:.1f}")
        print(f"  FPS Avg: {perf['fps_avg']:.1f} (target: 90)")
        print(f"  Memory Peak: {perf['memory_peak_mb']:.1f} MB (target: 8192)")
        
        # Errors and warnings
        if self.results["errors"]:
            print(f"\n❌ Errors ({len(self.results['errors'])}):")
            for error in self.results["errors"]:
                print(f"  - {error}")
        
        if self.results["warnings"]:
            print(f"\n⚠️  Warnings ({len(self.results['warnings'])}):"
            for warning in self.results["warnings"]:
                print(f"  - {warning}")
        
        # Recommendations
        self.generate_recommendations()
    
    def generate_recommendations(self):
        """Generate actionable recommendations based on results"""
        print(f"\n=== Recommendations ===")
        
        perf = self.results["performance"]
        
        # FPS recommendations
        if perf["fps_avg"] < 85:
            print("🔧 CRITICAL: FPS too low. Recommend:")
            print("  - Profile with Unreal Insights")
            print("  - Optimize spatial partitioning")
            print("  - Implement LOD system")
        elif perf["fps_avg"] < 90:
            print("⚠️  WARNING: FPS below target. Recommend:")
            print("  - Minor optimizations in tick functions")
            print("  - Reduce update frequency for distant objects")
        
        # Memory recommendations
        if perf["memory_peak_mb"] > 9000:
            print("🔧 CRITICAL: Memory too high. Recommend:")
            print("  - Profile memory allocations")
            print("  - Implement object pooling")
            print("  - Reduce texture quality")
        elif perf["memory_peak_mb"] > 8192:
            print("⚠️  WARNING: Memory above target. Recommend:")
            print("  - Optimize asset loading")
            print("  - Reduce concurrent systems")
        
        # System-specific recommendations
        for system in self.results["system_results"]:
            if not system["passed"]:
                print(f"🔧 FIX NEEDED: {system['system']} failed")
                print(f"  - Review {system['system']} implementation")
                print(f"  - Check for null pointers or uninitialized variables")
        
        if self.results["overall_success"]:
            print("\n✅ READY FOR RELEASE: All systems functional")
        else:
            print("\n❌ NEEDS FIXES: Address issues before release")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        report_path = sys.argv[1]
    else:
        report_path = "verification_report.json"
    
    analyzer = TestResultsAnalyzer(report_path)
    analyzer.analyze()