#!/usr/bin/env python3
"""
Autonomous Playtesting System Deployment Script
Deploys and configures the complete playtesting system with prerequisites checking.
"""

import sys
import os
import json
import subprocess
import platform
import socket
import shutil
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import importlib.util

class DeploymentError(Exception):
    """Custom exception for deployment errors"""
    pass

class PlaytestingSystemDeployer:
    def __init__(self, config_path: str = "deployment_config.json"):
        self.config_path = config_path
        self.config = {}
        self.deployment_report = {
            "timestamp": "",
            "status": "in_progress",
            "prerequisites": {},
            "directories": {},
            "configuration": {},
            "services": {},
            "errors": []
        }
        
    def log(self, message: str, level: str = "INFO"):
        """Log message with timestamp"""
        timestamp = subprocess.check_output(['date', '+%Y-%m-%d %H:%M:%S']).decode().strip()
        print(f"[{timestamp}] [{level}] {message}")
        
    def check_python_version(self) -> bool:
        """Check if Python version is compatible"""
        version = sys.version_info
        if version.major >= 3 and version.minor >= 8:
            self.deployment_report["prerequisites"]["python_version"] = {
                "status": "pass",
                "version": f"{version.major}.{version.minor}.{version.micro}"
            }
            self.log(f"Python version {version.major}.{version.minor}.{version.micro} is compatible")
            return True
        else:
            self.deployment_report["prerequisites"]["python_version"] = {
                "status": "fail",
                "version": f"{version.major}.{version.minor}.{version.micro}",
                "error": "Python 3.8+ required"
            }
            self.log("Python 3.8+ required", "ERROR")
            return False
            
    def check_tesseract(self) -> bool:
        """Check if Tesseract OCR is installed"""
        try:
            result = subprocess.run(['tesseract', '--version'],
                                  capture_output=True, text=True)
            if result.returncode == 0:
                version_line = result.stdout.split('\n')[0]
                self.deployment_report["prerequisites"]["tesseract"] = {
                    "status": "pass",
                    "version": version_line
                }
                self.log(f"Tesseract OCR found: {version_line}")
                return True
            else:
                raise FileNotFoundError
        except FileNotFoundError:
            self.deployment_report["prerequisites"]["tesseract"] = {
                "status": "warning",
                "error": "Tesseract OCR not found. Install from https://github.com/UB-Mannheim/tesseract/wiki",
                "note": "Tesseract is required for text recognition in screenshots. Some features will be limited without it."
            }
            self.log("Tesseract OCR not installed - visual text analysis will be limited", "WARNING")
            return True  # Don't fail deployment, just warn
            
    def check_dependencies(self) -> bool:
        """Check and install Python dependencies"""
        required_packages = [
            "flask", "requests", "pillow", "numpy", "opencv-python",
            "psutil", "pytesseract", "matplotlib", "pandas"
        ]
        
        missing_packages = []
        for package in required_packages:
            try:
                importlib.import_module(package.replace('-', '_'))
                self.log(f"Package {package} is installed")
            except ImportError:
                missing_packages.append(package)
                
        if missing_packages:
            self.log(f"Installing missing packages: {', '.join(missing_packages)}")
            try:
                subprocess.check_call([sys.executable, "-m", "pip", "install", *missing_packages])
                self.deployment_report["prerequisites"]["dependencies"] = {
                    "status": "pass",
                    "installed": missing_packages
                }
                return True
            except subprocess.CalledProcessError as e:
                self.deployment_report["prerequisites"]["dependencies"] = {
                    "status": "fail",
                    "error": f"Failed to install packages: {e}"
                }
                self.log(f"Failed to install packages: {e}", "ERROR")
                return False
        else:
            self.deployment_report["prerequisites"]["dependencies"] = {
                "status": "pass",
                "message": "All required packages are installed"
            }
            return True
            
    def create_directory_structure(self) -> bool:
        """Create required directory structure"""
        directories = [
            "screenshots_visual_playtest",
            "analysis_results",
            "verification_reports",
            "automation_config",
            "capture_output"
        ]
        
        success = True
        for directory in directories:
            try:
                Path(directory).mkdir(exist_ok=True)
                self.deployment_report["directories"][directory] = {
                    "status": "created",
                    "path": str(Path(directory).absolute())
                }
                self.log(f"Directory created/verified: {directory}")
            except Exception as e:
                self.deployment_report["directories"][directory] = {
                    "status": "fail",
                    "error": str(e)
                }
                self.log(f"Failed to create directory {directory}: {e}", "ERROR")
                success = False
                
        return success
        
    def validate_configuration(self) -> bool:
        """Validate configuration files"""
        config_files = [
            "playtesting_config.json",
            "flight_physics_presets.json",
            "deployment_config.json"
        ]
        
        success = True
        for config_file in config_files:
            if Path(config_file).exists():
                try:
                    with open(config_file, 'r') as f:
                        json.load(f)
                    self.deployment_report["configuration"][config_file] = {
                        "status": "valid",
                        "path": str(Path(config_file).absolute())
                    }
                    self.log(f"Configuration file valid: {config_file}")
                except json.JSONDecodeError as e:
                    self.deployment_report["configuration"][config_file] = {
                        "status": "invalid",
                        "error": f"Invalid JSON: {e}"
                    }
                    self.log(f"Invalid JSON in {config_file}: {e}", "ERROR")
                    success = False
            else:
                self.deployment_report["configuration"][config_file] = {
                    "status": "missing",
                    "error": "File not found"
                }
                self.log(f"Configuration file missing: {config_file}", "WARNING")
                
        return success
        
    def check_unreal_engine(self) -> bool:
        """Check Unreal Engine installation and project accessibility"""
        # Check for common Unreal Engine locations
        unreal_paths = [
            "C:/Program Files/Epic Games/UE_5.3",
            "C:/Program Files/Epic Games/UE_5.2",
            "C:/Program Files/Epic Games/UE_5.1",
            "C:/Program Files/Epic Games/UE_5.0"
        ]
        
        ue_found = False
        for ue_path in unreal_paths:
            if Path(ue_path).exists():
                self.deployment_report["prerequisites"]["unreal_engine"] = {
                    "status": "pass",
                    "path": ue_path
                }
                self.log(f"Unreal Engine found at: {ue_path}")
                ue_found = True
                break
                
        if not ue_found:
            self.deployment_report["prerequisites"]["unreal_engine"] = {
                "status": "warning",
                "message": "Unreal Engine not found in standard locations"
            }
            self.log("Unreal Engine not found in standard locations", "WARNING")
            
        # Check project files
        project_files = list(Path(".").glob("*.uproject"))
        if project_files:
            self.deployment_report["prerequisites"]["project"] = {
                "status": "pass",
                "projects": [str(p.absolute()) for p in project_files]
            }
            self.log(f"Unreal project files found: {len(project_files)}")
        else:
            self.deployment_report["prerequisites"]["project"] = {
                "status": "warning",
                "message": "No .uproject files found in current directory"
            }
            self.log("No .uproject files found", "WARNING")
            
        return True  # Don't fail deployment if UE not found, just warn
        
    def check_network_ports(self) -> bool:
        """Check if required network ports are available"""
        ports = [8080, 8081, 8082]  # game, analysis, dashboard
        port_status = {}
        
        for port in ports:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                result = sock.connect_ex(('localhost', port))
                sock.close()
                
                if result == 0:
                    port_status[str(port)] = {
                        "status": "in_use",
                        "message": f"Port {port} is already in use"
                    }
                    self.log(f"Port {port} is already in use", "WARNING")
                else:
                    port_status[str(port)] = {
                        "status": "available",
                        "message": f"Port {port} is available"
                    }
                    self.log(f"Port {port} is available")
            except Exception as e:
                port_status[str(port)] = {
                    "status": "error",
                    "error": str(e)
                }
                self.log(f"Error checking port {port}: {e}", "ERROR")
                
        self.deployment_report["services"]["ports"] = port_status
        return True  # Don't fail deployment if ports are in use
        
    def setup_environment_variables(self) -> bool:
        """Setup environment variables"""
        try:
            # Set TESSDATA_PREFIX if not set
            if platform.system() == "Windows":
                tesseract_path = "C:/Program Files/Tesseract-OCR/tessdata"
                if not os.environ.get("TESSDATA_PREFIX"):
                    os.environ["TESSDATA_PREFIX"] = tesseract_path
                    self.log(f"Set TESSDATA_PREFIX to {tesseract_path}")
                    
            self.deployment_report["configuration"]["environment"] = {
                "status": "configured",
                "variables": ["TESSDATA_PREFIX"]
            }
            return True
        except Exception as e:
            self.deployment_report["configuration"]["environment"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Failed to setup environment variables: {e}", "ERROR")
            return False
            
    def generate_deployment_report(self):
        """Generate deployment report"""
        import datetime
        self.deployment_report["timestamp"] = datetime.datetime.now().isoformat()
        
        # Determine overall status
        failed_checks = []
        for category, items in self.deployment_report.items():
            if isinstance(items, dict):
                for item, status in items.items():
                    if isinstance(status, dict) and status.get("status") in ["fail", "error"]:
                        failed_checks.append(f"{category}.{item}")
                        
        if failed_checks:
            self.deployment_report["status"] = "partial_success"
            self.deployment_report["errors"] = failed_checks
        else:
            self.deployment_report["status"] = "success"
            
        # Save report
        report_path = "verification_reports/deployment_report.json"
        with open(report_path, 'w') as f:
            json.dump(self.deployment_report, f, indent=2)
            
        self.log(f"Deployment report saved to: {report_path}")
        
    def deploy(self) -> bool:
        """Main deployment function"""
        self.log("Starting Playtesting System Deployment")
        
        try:
            # Run all checks
            checks = [
                ("Python Version", self.check_python_version),
                ("Tesseract OCR", self.check_tesseract),
                ("Dependencies", self.check_dependencies),
                ("Directory Structure", self.create_directory_structure),
                ("Configuration", self.validate_configuration),
                ("Unreal Engine", self.check_unreal_engine),
                ("Network Ports", self.check_network_ports),
                ("Environment Variables", self.setup_environment_variables)
            ]
            
            all_passed = True
            for check_name, check_func in checks:
                self.log(f"Running check: {check_name}")
                if not check_func():
                    if check_name in ["Python Version", "Tesseract OCR", "Dependencies"]:
                        all_passed = False
                        self.log(f"Critical check failed: {check_name}", "ERROR")
                        
            # Generate report
            self.generate_deployment_report()
            
            if all_passed:
                self.log("Deployment completed successfully!", "SUCCESS")
                return True
            else:
                self.log("Deployment completed with critical errors", "ERROR")
                return False
                
        except Exception as e:
            self.log(f"Deployment failed with exception: {e}", "ERROR")
            self.deployment_report["errors"].append(str(e))
            self.generate_deployment_report()
            return False

def main():
    """Main entry point"""
    deployer = PlaytestingSystemDeployer()
    success = deployer.deploy()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()