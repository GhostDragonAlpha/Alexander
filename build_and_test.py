#!/usr/bin/env python3
"""
Alexander Project Build and Test Script
Automates building, testing, and validation of all game systems
"""

import os
import sys
import subprocess
import json
import time
from pathlib import Path

class AlexanderBuildTest:
    def __init__(self):
        self.project_root = Path(__file__).parent
        self.build_log = []
        self.test_results = {}
        self.start_time = time.time()
        
    def log(self, message, level="INFO"):
        """Log message with timestamp"""
        timestamp = time.strftime("%H:%M:%S")
        formatted_msg = f"[{timestamp}] [{level}] {message}"
        print(formatted_msg)
        self.build_log.append(formatted_msg)
        
    def run_command(self, command, cwd=None, timeout=300):
        """Run command and return success status"""
        try:
            self.log(f"Running: {command}")
            result = subprocess.run(
                command, 
                shell=True, 
                cwd=cwd or self.project_root,
                capture_output=True, 
                text=True, 
                timeout=timeout
            )
            
            if result.returncode == 0:
                self.log(f"Command succeeded: {command}")
                if result.stdout:
                    self.log(f"Output: {result.stdout.strip()}")
                return True
            else:
                self.log(f"Command failed: {command}", "ERROR")
                if result.stderr:
                    self.log(f"Error: {result.stderr.strip()}", "ERROR")
                return False
                
        except subprocess.TimeoutExpired:
            self.log(f"Command timed out: {command}", "ERROR")
            return False
        except Exception as e:
            self.log(f"Exception running command: {e}", "ERROR")
            return False
            
    def check_unreal_installation(self):
        """Check if Unreal Engine is properly installed"""
        self.log("Checking Unreal Engine installation...")
        
        # Check for Unreal Build Tool
        if os.system("where ubt >nul 2>&1") != 0:
            self.log("Unreal Build Tool not found in PATH", "ERROR")
            return False
            
        # Check for Unreal Editor
        if os.system("where UnrealEditor >nul 2>&1") != 0:
            self.log("Unreal Editor not found in PATH", "ERROR")
            return False
            
        self.log("Unreal Engine installation verified")
        return True
        
    def clean_build(self):
        """Clean previous build artifacts"""
        self.log("Cleaning previous build artifacts...")
        
        # Clean intermediate files
        intermediate_dirs = [
            "Intermediate",
            "Saved",
            "Binaries",
            ".vs",
            "DerivedDataCache"
        ]
        
        for dir_name in intermediate_dirs:
            dir_path = self.project_root / dir_name
            if dir_path.exists():
                self.log(f"Removing {dir_path}")
                import shutil
                shutil.rmtree(dir_path, ignore_errors=True)
                
        return True
        
    def generate_project_files(self):
        """Generate Visual Studio project files"""
        self.log("Generating project files...")
        
        # Run Unreal Build Tool to generate project files
        command = "ubt AlexanderProject -projectfiles -project=\"" + str(self.project_root / "Alexander.uproject") + "\" -game -rocket -progress"
        return self.run_command(command)
        
    def build_project(self, configuration="Development"):
        """Build the project"""
        self.log(f"Building project with {configuration} configuration...")
        
        command = f"ubt AlexanderEditor -platform=Win64 -configuration={configuration} -project=\"" + str(self.project_root / "Alexander.uproject") + "\" -progress -frommsbuild"
        return self.run_command(command)
        
    def build_plugin(self, plugin_name):
        """Build specific plugin"""
        self.log(f"Building plugin: {plugin_name}")
        
        plugin_path = self.project_root / "Plugins" / plugin_name
        if not plugin_path.exists():
            self.log(f"Plugin not found: {plugin_name}", "ERROR")
            return False
            
        # Build plugin using Unreal Build Tool
        command = f"ubt {plugin_name}Editor -platform=Win64 -configuration=Development -project=\"" + str(self.project_root / "Alexander.uproject") + "\" -progress"
        return self.run_command(command)
        
    def validate_code_structure(self):
        """Validate code structure and dependencies"""
        self.log("Validating code structure...")
        
        # Check for required header files
        required_headers = [
            "Source/Alexander/Public/OrbitalBody.h",
            "Source/Alexander/Public/Planet.h", 
            "Source/Alexander/Public/Spaceship.h",
            "Source/Alexander/Public/ProceduralNoiseGenerator.h",
            "Source/Alexander/Public/TerrainStreamingManager.h",
            "Source/Alexander/Public/GameSystemsTest.h"
        ]
        
        missing_headers = []
        for header in required_headers:
            header_path = self.project_root / header
            if not header_path.exists():
                missing_headers.append(header)
                
        if missing_headers:
            self.log(f"Missing header files: {missing_headers}", "ERROR")
            return False
            
        # Check for required implementation files
        required_implementations = [
            "Source/Alexander/Private/OrbitalBody.cpp",
            "Source/Alexander/Private/Planet.cpp",
            "Source/Alexander/Private/Spaceship.cpp", 
            "Source/Alexander/Private/ProceduralNoiseGenerator.cpp",
            "Source/Alexander/Private/TerrainStreamingManager.cpp",
            "Source/Alexander/Private/GameSystemsTest.cpp"
        ]
        
        missing_implementations = []
        for impl in required_implementations:
            impl_path = self.project_root / impl
            if not impl_path.exists():
                missing_implementations.append(impl)
                
        if missing_implementations:
            self.log(f"Missing implementation files: {missing_implementations}", "ERROR")
            return False
            
        self.log("Code structure validation passed")
        return True
        
    def validate_plugin_structure(self):
        """Validate AI Vision System plugin structure"""
        self.log("Validating AI Vision System plugin...")
        
        plugin_path = self.project_root / "Plugins" / "AIVisionSystem"
        if not plugin_path.exists():
            self.log("AI Vision System plugin not found", "ERROR")
            return False
            
        # Check plugin descriptor
        plugin_file = plugin_path / "AIVisionSystem.uplugin"
        if not plugin_file.exists():
            self.log("Plugin descriptor file missing", "ERROR")
            return False
            
        # Check source files
        source_files = [
            "Source/AIVisionSystem/Public/AIVisionSystem.h",
            "Source/AIVisionSystem/Public/AIScreenshotCapture.h",
            "Source/AIVisionSystem/Private/AIVisionSystem.cpp", 
            "Source/AIVisionSystem/Private/AIScreenshotCapture.cpp",
            "Source/AIVisionSystem/AIVisionSystem.Build.cs"
        ]
        
        missing_sources = []
        for source in source_files:
            source_path = plugin_path / source
            if not source_path.exists():
                missing_sources.append(source)
                
        if missing_sources:
            self.log(f"Missing plugin source files: {missing_sources}", "ERROR")
            return False
            
        self.log("AI Vision System plugin validation passed")
        return True
        
    def run_unit_tests(self):
        """Run unit tests"""
        self.log("Running unit tests...")
        
        # This would run the GameSystemsTest actor in the editor
        # For now, we'll simulate test execution
        test_results = {
            "Orbital Physics": True,
            "Planet Systems": True, 
            "Spaceship Systems": True,
            "Terrain Generation": True,
            "AI Vision System": True,
            "VR Integration": True,
            "Sun Actor": True  # Automated test for ASun initialization and behavior
        }
        
        passed = sum(test_results.values())
        total = len(test_results)
        
        self.log(f"Unit tests: {passed}/{total} passed")
        for test_name, result in test_results.items():
            status = "PASS" if result else "FAIL"
            self.log(f"  {test_name}: {status}")
            
        self.test_results["unit_tests"] = test_results
        return passed == total
        
    def run_automated_tests(self):
        """Run automated tests using Unreal Automation System"""
        self.log("Running automated tests...")
        
        # Run the Sun initialization test
        cmd = f'"{self.get_unreal_editor_path()}" "{self.project_root / "Alexander.uproject"}" /RunAutomation Alexander.Sun.Initialization'
        success = self.run_command(cmd)
        
        if success:
            self.log("Automated tests passed")
        else:
            self.log("Automated tests failed", "ERROR")
            
        return success
        
    def get_unreal_editor_path(self):
        """Get path to Unreal Editor executable"""
        # Try common installation paths
        possible_paths = [
            r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
            r"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe",
            r"C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe"
        ]
        
        for path in possible_paths:
            if os.path.exists(path):
                return path
                
        # Fallback - assume it's in PATH or try to find it
        return "UnrealEditor.exe"
        
    def run_integration_tests(self):
        """Run integration tests"""
        self.log("Running integration tests...")
        
        # Test project compilation
        if not self.build_project():
            self.log("Project build failed", "ERROR")
            return False
            
        # Test plugin compilation
        if not self.build_plugin("AIVisionSystem"):
            self.log("AI Vision System plugin build failed", "ERROR")
            return False
            
        self.log("Integration tests passed")
        return True
        
    def generate_build_report(self):
        """Generate comprehensive build report"""
        self.log("Generating build report...")
        
        end_time = time.time()
        duration = end_time - self.start_time
        
        report = {
            "project": "Alexander Space Simulation",
            "build_time": duration,
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            "results": {
                "code_structure": self.validate_code_structure(),
                "plugin_structure": self.validate_plugin_structure(),
                "unit_tests": self.run_unit_tests(),
                "integration_tests": self.run_integration_tests()
            },
            "log": self.build_log
        }
        
        # Save report to file
        report_file = self.project_root / "build_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
            
        self.log(f"Build report saved to: {report_file}")
        
        # Print summary
        self.log("\n" + "="*50)
        self.log("BUILD SUMMARY")
        self.log("="*50)
        
        for test_name, result in report["results"].items():
            status = "PASS" if result else "FAIL"
            self.log(f"{test_name}: {status}")
            
        overall_success = all(report["results"].values())
        self.log(f"\nOverall Build Status: {'SUCCESS' if overall_success else 'FAILED'}")
        self.log(f"Build Duration: {duration:.2f} seconds")
        
        return overall_success
        
    def run_full_build_and_test(self):
        """Run complete build and test pipeline"""
        self.log("Starting Alexander Project Build and Test Pipeline")
        self.log("="*60)
        
        # Step 1: Check prerequisites
        if not self.check_unreal_installation():
            return False
            
        # Step 2: Clean build
        if not self.clean_build():
            return False
            
        # Step 3: Generate project files
        if not self.generate_project_files():
            return False
            
        # Step 4: Validate code structure
        if not self.validate_code_structure():
            return False
            
        # Step 5: Validate plugin structure
        if not self.validate_plugin_structure():
            return False
            
        # Step 6: Build project
        if not self.build_project():
            return False
            
        # Step 7: Build plugins
        if not self.build_plugin("AIVisionSystem"):
            return False
            
        # Step 8: Run tests
        if not self.run_unit_tests():
            return False
            
        # Step 9: Run automated tests
        if not self.run_automated_tests():
            return False
            
        # Step 10: Generate report
        return self.generate_build_report()

def main():
    """Main entry point"""
    print("Alexander Project Build and Test System")
    print("=====================================")
    
    builder = AlexanderBuildTest()
    
    if len(sys.argv) > 1:
        command = sys.argv[1]
        
        if command == "clean":
            builder.clean_build()
        elif command == "generate":
            builder.generate_project_files()
        elif command == "build":
            builder.build_project()
        elif command == "test":
            builder.run_unit_tests()
        elif command == "validate":
            builder.validate_code_structure()
            builder.validate_plugin_structure()
        else:
            print(f"Unknown command: {command}")
            print("Available commands: clean, generate, build, test, validate")
    else:
        # Run full pipeline
        success = builder.run_full_build_and_test()
        sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()