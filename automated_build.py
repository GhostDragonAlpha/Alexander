"""
UnrealFrontend Automation Script for Alexander Project
Provides command-line automation for building, cooking, and packaging
"""

import subprocess
import os
import sys
from datetime import datetime
import argparse

class UnrealFrontendAutomation:
    def __init__(self):
        self.ue_path = r"C:\Program Files\Epic Games\UE_5.6"
        self.frontend_exe = os.path.join(self.ue_path, "Engine", "Binaries", "Win64", "UnrealFrontend.exe")
        self.project_path = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
        self.builds_dir = r"C:\Users\allen\Desktop\Alexander\Builds"
        
        # Ensure builds directory exists
        os.makedirs(self.builds_dir, exist_ok=True)
        
    def build_project(self, configuration="Development"):
        """Compile C++ code only"""
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Building Alexander ({configuration})...")
        
        cmd = [
            f'"{self.frontend_exe}"',
            f'-project="{self.project_path}"',
            '-Build',
            f'-Configuration={configuration}',
            '-Platform=Win64',
            '-unattended',
            '-nopause'
        ]
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Build succeeded")
            return True
        else:
            print(f"✗ Build failed")
            if result.stderr:
                print(result.stderr)
            return False
    
    def cook_content(self):
        """Cook game content for Win64"""
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Cooking content...")
        
        cmd = [
            f'"{self.frontend_exe}"',
            f'-project="{self.project_path}"',
            '-Cook',
            '-TargetPlatform=Win64',
            '-unattended',
            '-nopause'
        ]
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Cook succeeded")
            return True
        else:
            print(f"✗ Cook failed")
            if result.stderr:
                print(result.stderr)
            return False
    
    def package_game(self, configuration="Shipping", build_name=None):
        """Package complete game build"""
        if build_name is None:
            build_name = f"Alexander_{configuration}_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        
        staging_dir = os.path.join(self.builds_dir, build_name)
        os.makedirs(staging_dir, exist_ok=True)
        
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Packaging {configuration} build to {staging_dir}...")
        
        cmd = [
            f'"{self.frontend_exe}"',
            f'-project="{self.project_path}"',
            '-Package',
            f'-Configuration={configuration}',
            '-Platform=Win64',
            f'-StagingDirectory="{staging_dir}"',
            '-unattended',
            '-nopause'
        ]
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Package succeeded: {staging_dir}")
            return True, staging_dir
        else:
            print(f"✗ Package failed")
            if result.stderr:
                print(result.stderr)
            return False, None
    
    def full_build_pipeline(self, configuration="Development", incremental=False):
        """Build, cook, and package in one go"""
        build_name = f"Alexander_{configuration}_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        staging_dir = os.path.join(self.builds_dir, build_name)
        os.makedirs(staging_dir, exist_ok=True)
        
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Starting full build pipeline ({configuration})...")
        
        cmd = [
            f'"{self.frontend_exe}"',
            f'-project="{self.project_path}"',
            '-Build',
            '-Cook',
            '-Package',
            f'-Configuration={configuration}',
            '-Platform=Win64',
            f'-StagingDirectory="{staging_dir}"',
            '-Pak',
            '-Compressed',
        ]
        
        if incremental:
            cmd.append('-IterativeCooking')
        
        cmd.extend(['-unattended', '-nopause'])
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Full pipeline succeeded: {staging_dir}")
            return True, staging_dir
        else:
            print(f"✗ Full pipeline failed")
            if result.stderr:
                print(result.stderr)
            return False, None

def main():
    parser = argparse.ArgumentParser(description='Alexander Project Build Automation')
    parser.add_argument('action', choices=['build', 'cook', 'package', 'full', 'dev', 'ship'],
                       help='Build action to perform')
    parser.add_argument('--config', default='Development', 
                       choices=['Development', 'DebugGame', 'Shipping', 'Test'],
                       help='Build configuration')
    parser.add_argument('--incremental', action='store_true',
                       help='Use incremental cooking')
    
    args = parser.parse_args()
    
    automation = UnrealFrontendAutomation()
    
    success = False
    
    if args.action == 'build':
        success = automation.build_project(args.config)
    
    elif args.action == 'cook':
        success = automation.cook_content()
    
    elif args.action == 'package':
        success, _ = automation.package_game(args.config)
    
    elif args.action == 'full':
        success, _ = automation.full_build_pipeline(args.config, args.incremental)
    
    elif args.action == 'dev':
        success, _ = automation.full_build_pipeline('Development', args.incremental)
    
    elif args.action == 'ship':
        success, _ = automation.full_build_pipeline('Shipping', args.incremental)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("Alexander Project Build Automation")
        print("\nUsage:")
        print("  python automated_build.py build              # Compile only (Development)")
        print("  python automated_build.py build --config=Shipping  # Compile for shipping")
        print("  python automated_build.py cook               # Cook content only")
        print("  python automated_build.py package            # Package game")
        print("  python automated_build.py full               # Build + Cook + Package")
        print("  python automated_build.py dev                # Quick development build")
        print("  python automated_build.py ship               # Full shipping build")
        print("  python automated_build.py full --incremental # Incremental cooking")
        print("\nExamples:")
        print("  python automated_build.py build")
        print("  python automated_build.py ship")
        print("  python automated_build.py full --config=Development --incremental")
        sys.exit(0)
    
    main()
