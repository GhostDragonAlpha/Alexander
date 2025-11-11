#!/usr/bin/env python3
"""
First Run Script
Guides users through initial system run with interactive tutorial.
"""

import sys
import os
import json
import time
import requests
import subprocess
from pathlib import Path
from typing import Dict, List, Optional
from datetime import datetime
import webbrowser

class FirstRunError(Exception):
    """Custom exception for first run errors"""
    pass

class FirstRunGuide:
    def __init__(self, config_path: str = "deployment_config.json"):
        self.config_path = config_path
        self.config = self.load_config()
        self.tutorial_steps = []
        self.current_step = 0
        self.base_url = "http://localhost"
        self.first_run_report = {
            "timestamp": datetime.now().isoformat(),
            "status": "in_progress",
            "tutorial_completed": False,
            "configuration_done": False,
            "test_scenario_run": False,
            "user_understanding": False,
            "errors": []
        }
        
    def log(self, message: str, level: str = "INFO"):
        """Log message with timestamp"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] [{level}] {message}")
        
    def load_config(self) -> Dict:
        """Load deployment configuration"""
        try:
            with open(self.config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            self.log(f"Config file not found: {self.config_path}", "WARNING")
            return {}
            
    def print_header(self, title: str):
        """Print formatted header"""
        print("\n" + "="*60)
        print(f"  {title}")
        print("="*60 + "\n")
        
    def print_step(self, step_number: int, title: str, description: str):
        """Print formatted step"""
        print(f"\n📋 STEP {step_number}: {title}")
        print(f"   {description}")
        print("-" * 60)
        
    def wait_for_user(self, prompt: str = "Press Enter to continue..."):
        """Wait for user input"""
        input(f"\n{prompt}")
        
    def check_system_status(self) -> bool:
        """Check if system is ready for first run"""
        self.print_header("SYSTEM STATUS CHECK")
        
        services = [
            ("Game Service", 8080),
            ("Analysis Service", 8081),
            ("Dashboard Service", 8082)
        ]
        
        all_ready = True
        for service_name, port in services:
            try:
                url = f"{self.base_url}:{port}/health"
                response = requests.get(url, timeout=5)
                if response.status_code == 200:
                    print(f"✅ {service_name} is ready")
                else:
                    print(f"❌ {service_name} is not responding properly")
                    all_ready = False
            except:
                print(f"❌ {service_name} is not accessible")
                all_ready = False
                
        if not all_ready:
            print("\n⚠️  Some services are not ready. Please start all services first.")
            print("   Run: python launch_autonomous_testing.py")
            return False
            
        print("\n✅ All systems are ready for first run!")
        return True
        
    def guided_configuration(self):
        """Guide user through configuration"""
        self.print_header("GUIDED CONFIGURATION")
        
        self.print_step(1, "Review Configuration", 
                       "Let's review your current configuration.")
        self.wait_for_user()
        
        # Show current configuration
        config_files = ["playtesting_config.json", "deployment_config.json"]
        for config_file in config_files:
            if Path(config_file).exists():
                print(f"\n📄 {config_file}:")
                try:
                    with open(config_file, 'r') as f:
                        config_data = json.load(f)
                        print(json.dumps(config_data, indent=2))
                except:
                    print("   Unable to read configuration file")
                    
        self.print_step(2, "Configuration Validation", 
                       "Validating your configuration...")
        self.wait_for_user()
        
        # Run validation
        try:
            result = subprocess.run([sys.executable, "validate_deployment.py"], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                print("✅ Configuration validation passed!")
                self.first_run_report["configuration_done"] = True
            else:
                print("❌ Configuration validation failed:")
                print(result.stderr)
        except Exception as e:
            print(f"❌ Configuration validation error: {e}")
            
        self.wait_for_user("Configuration review complete. Press Enter to continue...")
        
    def run_test_scenario(self):
        """Run a short test scenario"""
        self.print_header("TEST SCENARIO")
        
        print("""
🎮 TEST SCENARIO: Basic Playtesting Loop
        
This test will run a complete playtesting cycle:
1. Capture a screenshot from the game
2. Analyze the screenshot for issues
3. Collect performance data
4. Generate a report
5. Display results in dashboard

This will take approximately 2-3 minutes.
        """)
        
        self.wait_for_user("Ready to start the test scenario? Press Enter...")
        
        try:
            print("\n🚀 Starting test scenario...")
            
            # Step 1: Capture screenshot
            print("\n📸 Step 1: Capturing screenshot...")
            screenshot_url = f"{self.base_url}:8080/api/screenshot"
            response = requests.post(screenshot_url, json={}, timeout=30)
            
            if response.status_code == 200:
                screenshot_data = response.json()
                print(f"✅ Screenshot captured: {screenshot_data['screenshot_path']}")
            else:
                raise FirstRunError("Screenshot capture failed")
                
            # Step 2: Analyze screenshot
            print("\n🔍 Step 2: Analyzing screenshot...")
            analysis_url = f"{self.base_url}:8081/api/analyze"
            response = requests.post(analysis_url, json={
                "screenshot_path": screenshot_data["screenshot_path"]
            }, timeout=60)
            
            if response.status_code == 200:
                analysis_data = response.json()
                print(f"✅ Analysis complete, found {len(analysis_data.get('issues', []))} issues")
            else:
                raise FirstRunError("Analysis failed")
                
            # Step 3: Collect performance data
            print("\n📊 Step 3: Collecting performance data...")
            performance_url = f"{self.base_url}:8080/api/performance"
            response = requests.get(performance_url, timeout=10)
            
            if response.status_code == 200:
                performance_data = response.json()
                print(f"✅ Performance data collected: {len(performance_data)} metrics")
            else:
                raise FirstRunError("Performance collection failed")
                
            # Step 4: Get issues report
            print("\n📋 Step 4: Generating issues report...")
            issues_url = f"{self.base_url}:8081/api/issues"
            response = requests.get(issues_url, timeout=10)
            
            if response.status_code == 200:
                issues_data = response.json()
                print(f"✅ Issues report generated: {len(issues_data.get('issues', []))} total issues")
            else:
                raise FirstRunError("Issues report generation failed")
                
            # Step 5: Check dashboard
            print("\n📈 Step 5: Checking dashboard...")
            dashboard_url = f"{self.base_url}:8082/api/status"
            response = requests.get(dashboard_url, timeout=10)
            
            if response.status_code == 200:
                print("✅ Dashboard is accessible and displaying data")
            else:
                raise FirstRunError("Dashboard access failed")
                
            print("\n🎉 Test scenario completed successfully!")
            self.first_run_report["test_scenario_run"] = True
            
            # Show summary
            print(f"\n📊 TEST SUMMARY:")
            print(f"   - Screenshot captured and analyzed")
            print(f"   - {len(analysis_data.get('issues', []))} issues detected")
            print(f"   - {len(performance_data)} performance metrics collected")
            print(f"   - Dashboard updated with results")
            
        except Exception as e:
            print(f"\n❌ Test scenario failed: {e}")
            self.first_run_report["errors"].append(str(e))
            
        self.wait_for_user("Test scenario complete. Press Enter to continue...")
        
    def interpret_results_tutorial(self):
        """Show user how to interpret results"""
        self.print_header("INTERPRETING RESULTS")
        
        print("""
📊 UNDERSTANDING YOUR PLAYTESTING RESULTS

1. **ISSUE CATEGORIES:**
   - 🐛 Bugs: Functional problems that need fixing
   - ⚠️  Warnings: Potential issues to monitor
   - 💡 Suggestions: Improvements for better gameplay

2. **PERFORMANCE METRICS:**
   - 🎯 FPS: Frames per second (higher is better)
   - ⏱️  Frame Time: Time to render each frame (lower is better)
   - 🖥️  CPU Usage: Processor utilization (lower is better)
   - 💾 Memory Usage: RAM consumption (monitor for leaks)

3. **ISSUE PRIORITY:**
   - 🔴 Critical: Must fix immediately
   - 🟡 Medium: Fix when possible
   - 🟢 Low: Nice to have improvements

4. **DASHBOARD VIEWS:**
   - 📈 Real-time metrics
   - 📊 Historical trends
   - 🚨 Active issues
   - ✅ Recent fixes
        """)
        
        self.wait_for_user("Results tutorial complete. Press Enter to continue...")
        
        # Show example results if available
        try:
            issues_url = f"{self.base_url}:8081/api/issues"
            response = requests.get(issues_url, timeout=10)
            
            if response.status_code == 200:
                issues_data = response.json()
                if issues_data.get("issues"):
                    print(f"\n📋 EXAMPLE ISSUES FOUND ({len(issues_data['issues'])} total):")
                    for i, issue in enumerate(issues_data["issues"][:5], 1):  # Show first 5
                        priority_emoji = "🔴" if issue.get("priority") == "critical" else "🟡"
                        print(f"   {priority_emoji} {i}. {issue.get('title', 'Unknown')}")
                        print(f"      Category: {issue.get('category', 'unknown')}")
                        print(f"      Location: {issue.get('location', 'unknown')}")
        except:
            print("\nℹ️  No example issues available yet")
            
        self.wait_for_user("Press Enter to continue...")
        
    def interactive_tutorial(self):
        """Run interactive tutorial"""
        self.print_header("INTERACTIVE TUTORIAL")
        
        tutorial_steps = [
            {
                "title": "System Overview",
                "description": "The playtesting system automatically captures gameplay data, analyzes it for issues, and provides actionable recommendations.",
                "action": self.show_system_overview
            },
            {
                "title": "Daily Workflow", 
                "description": "Learn how to use the system in your daily development workflow.",
                "action": self.show_daily_workflow
            },
            {
                "title": "Best Practices",
                "description": "Tips and best practices for getting the most out of the system.",
                "action": self.show_best_practices
            },
            {
                "title": "Troubleshooting",
                "description": "Common issues and how to resolve them.",
                "action": self.show_troubleshooting
            }
        ]
        
        for i, step in enumerate(tutorial_steps, 1):
            self.print_step(i, step["title"], step["description"])
            step["action"]()
            self.wait_for_user(f"Step {i} complete. Press Enter for next step...")
            
        self.first_run_report["tutorial_completed"] = True
        
    def show_system_overview(self):
        """Show system overview"""
        print("""
🏗️  SYSTEM ARCHITECTURE:

┌─────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  Game Service   │────▶│ Analysis Service │────▶│ Dashboard Service│
│   (Port 8080)   │     │   (Port 8081)    │     │   (Port 8082)    │
└─────────────────┘     └──────────────────┘     └──────────────────┘
        │                       │                       │
        ▼                       ▼                       ▼
┌─────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  Screenshots    │     │   Issue Reports  │     │  Real-time Data  │
│   & Performance │     │  & Analysis      │     │   & Metrics      │
└─────────────────┘     └──────────────────┘     └──────────────────┘

🔧 KEY COMPONENTS:
• Game Service: Captures screenshots and performance data
• Analysis Service: Analyzes gameplay using AI/ML techniques
• Dashboard Service: Displays results and metrics in real-time
        """)
        
    def show_daily_workflow(self):
        """Show daily workflow"""
        print("""
📅 DAILY WORKFLOW:

Morning:
1. Start the playtesting system
   python launch_autonomous_testing.py

2. Begin your development work

3. Monitor dashboard for issues
   http://localhost:8082

Throughout the day:
• System automatically captures and analyzes gameplay
• Check dashboard periodically for new issues
• Review performance trends
• Address critical issues as they appear

End of day:
1. Review daily summary report
2. Check for any critical issues that need attention
3. Stop the system (if needed)
   python stop_all.py

4. Review generated reports in verification_reports/
        """)
        
    def show_best_practices(self):
        """Show best practices"""
        print("""
💡 BEST PRACTICES:

✅ DO:
• Run the system continuously during development
• Check dashboard regularly for new issues
• Address critical issues immediately
• Use performance data to optimize your game
• Review trends over time, not just single data points
• Keep the game running for best results

❌ DON'T:
• Ignore warnings - they often indicate real issues
• Run only short tests - continuous data is more valuable
• Focus only on FPS - consider all performance metrics
• Forget to check the dashboard regularly
• Ignore memory usage trends
• Run without proper configuration

🎯 PRO TIPS:
• Set up alerts for critical issues
• Review weekly summaries for patterns
• Use data to prioritize development tasks
• Share reports with your team
• Keep historical data for trend analysis
        """)
        
    def show_troubleshooting(self):
        """Show troubleshooting guide"""
        print("""
🔧 TROUBLESHOOTING GUIDE:

Problem: Services won't start
→ Check if ports 8080, 8081, 8082 are available
→ Verify Python 3.8+ is installed
→ Check that all dependencies are installed

Problem: Screenshots not capturing
→ Ensure game is running and visible
→ Check screen resolution settings
→ Verify screenshot directory permissions

Problem: Analysis not working
→ Check Tesseract OCR installation
→ Verify TESSDATA_PREFIX environment variable
→ Check analysis service logs

Problem: Dashboard not updating
→ Refresh the browser page
→ Check dashboard service is running
→ Verify network connectivity

Problem: High memory usage
→ This is normal during analysis
→ System automatically manages memory
→ Check for memory leaks if usage keeps growing

Need more help? Check the troubleshooting guide:
PLAYTESTING_TROUBLESHOOTING.md
        """)
        
    def generate_first_run_report(self):
        """Generate first run report"""
        # Determine overall success
        success = (self.first_run_report["tutorial_completed"] and 
                  self.first_run_report["configuration_done"] and 
                  self.first_run_report["test_scenario_run"])
        
        self.first_run_report["status"] = "completed" if success else "partial"
        
        # Save report
        report_path = "verification_reports/first_run_report.json"
        with open(report_path, 'w') as f:
            json.dump(self.first_run_report, f, indent=2)
            
        self.log(f"First run report saved to: {report_path}")
        
        # Show completion message
        self.print_header("FIRST RUN COMPLETE!")
        
        if success:
            print("""
🎉 CONGRATULATIONS!

You have successfully completed the first run tutorial and test scenario.
Your playtesting system is now ready for production use.

📋 SUMMARY:
✅ System status check completed
✅ Configuration validated
✅ Test scenario executed successfully
✅ Results interpretation tutorial completed
✅ Interactive tutorial completed

🚀 NEXT STEPS:
1. Start using the system in your daily workflow
2. Monitor the dashboard regularly
3. Address issues as they are detected
4. Review weekly summary reports

📖 RESOURCES:
• PLAYTESTING_USER_MANUAL.md - Complete user guide
• PLAYTESTING_TROUBLESHOOTING.md - Troubleshooting help
• verification_reports/ - Generated reports and certificates

Need help? Check the documentation or run this tutorial again.
            """)
        else:
            print("""
⚠️  FIRST RUN PARTIALLY COMPLETE

Some steps were not completed successfully. Please review the errors above
and try again. Common issues:

• Services not running - Start services first
• Configuration errors - Run validate_deployment.py
• Network issues - Check port availability

Run this script again after resolving any issues.
            """)
        
    def run_first_run(self):
        """Run the complete first run experience"""
        self.print_header("WELCOME TO AUTONOMOUS PLAYTESTING SYSTEM")
        
        print("""
🎮 FIRST RUN TUTORIAL

This interactive tutorial will guide you through:
1. System status check
2. Guided configuration review
3. Test scenario execution
4. Results interpretation
5. Interactive tutorial
6. Best practices and troubleshooting

Estimated time: 15-20 minutes

Let's get started!
        """)
        
        self.wait_for_user("Ready to begin? Press Enter...")
        
        # Check system status
        if not self.check_system_status():
            return False
            
        # Guided configuration
        self.guided_configuration()
        
        # Run test scenario
        self.run_test_scenario()
        
        # Results interpretation
        self.interpret_results_tutorial()
        
        # Interactive tutorial
        self.interactive_tutorial()
        
        # Mark user understanding as complete
        self.first_run_report["user_understanding"] = True
        
        # Generate report
        self.generate_first_run_report()
        
        return True

def main():
    """Main entry point"""
    guide = FirstRunGuide()
    success = guide.run_first_run()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()