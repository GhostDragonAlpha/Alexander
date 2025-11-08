"""
Nightly Build Scheduler for Alexander Project
Schedules automated builds at specified times
"""

import schedule
import time
from datetime import datetime
from automated_build import UnrealFrontendAutomation
import smtplib
from email.mime.text import MIMEText
import os

class NightlyBuildScheduler:
    def __init__(self, email_notifications=False):
        self.automation = UnrealFrontendAutomation()
        self.email_notifications = email_notifications
        self.log_dir = os.path.join(self.automation.builds_dir, "Logs")
        os.makedirs(self.log_dir, exist_ok=True)
        
    def log_message(self, message):
        """Log message to file and console"""
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        log_entry = f"[{timestamp}] {message}\n"
        
        print(log_entry.strip())
        
        log_file = os.path.join(self.log_dir, f"nightly_build_{datetime.now().strftime('%Y%m%d')}.log")
        with open(log_file, 'a') as f:
            f.write(log_entry)
    
    def send_notification(self, subject, body):
        """Send email notification (configure SMTP settings as needed)"""
        if not self.email_notifications:
            return
        
        # TODO: Configure your SMTP settings
        # msg = MIMEText(body)
        # msg['Subject'] = subject
        # msg['From'] = 'builds@alexander.local'
        # msg['To'] = 'dev-team@alexander.local'
        # 
        # with smtplib.SMTP('localhost') as smtp:
        #     smtp.send_message(msg)
        
        self.log_message(f"Notification: {subject}")
    
    def nightly_development_build(self):
        """Run automated nightly development build"""
        self.log_message("=" * 60)
        self.log_message("Starting Nightly Development Build")
        self.log_message("=" * 60)
        
        try:
            # Full development build with incremental cooking
            success, build_dir = self.automation.full_build_pipeline('Development', incremental=True)
            
            if success:
                self.log_message(f"✓ Nightly build completed successfully")
                self.log_message(f"Build location: {build_dir}")
                self.send_notification(
                    "Alexander Nightly Build - Success",
                    f"Development build completed successfully.\nLocation: {build_dir}"
                )
            else:
                self.log_message(f"✗ Nightly build failed")
                self.send_notification(
                    "Alexander Nightly Build - FAILED",
                    "Development build failed. Check logs for details."
                )
                
        except Exception as e:
            self.log_message(f"✗ Exception during build: {str(e)}")
            self.send_notification(
                "Alexander Nightly Build - ERROR",
                f"Exception occurred: {str(e)}"
            )
        
        self.log_message("Nightly build session completed")
        self.log_message("=" * 60)
    
    def weekly_shipping_build(self):
        """Run weekly shipping build"""
        self.log_message("=" * 60)
        self.log_message("Starting Weekly Shipping Build")
        self.log_message("=" * 60)
        
        try:
            # Full clean shipping build
            success, build_dir = self.automation.full_build_pipeline('Shipping', incremental=False)
            
            if success:
                self.log_message(f"✓ Weekly shipping build completed")
                self.log_message(f"Build location: {build_dir}")
                self.send_notification(
                    "Alexander Weekly Shipping Build - Success",
                    f"Shipping build completed.\nLocation: {build_dir}\nReady for distribution testing."
                )
            else:
                self.log_message(f"✗ Weekly shipping build failed")
                self.send_notification(
                    "Alexander Weekly Shipping Build - FAILED",
                    "Shipping build failed. Check logs for details."
                )
                
        except Exception as e:
            self.log_message(f"✗ Exception during build: {str(e)}")
            self.send_notification(
                "Alexander Weekly Shipping Build - ERROR",
                f"Exception occurred: {str(e)}"
            )
        
        self.log_message("Weekly build session completed")
        self.log_message("=" * 60)
    
    def quick_compile_check(self):
        """Quick compile-only check (no cooking/packaging)"""
        self.log_message("Running quick compile check...")
        
        try:
            success = self.automation.build_project('Development')
            
            if success:
                self.log_message("✓ Quick compile check passed")
            else:
                self.log_message("✗ Quick compile check failed")
                self.send_notification(
                    "Alexander Compile Check - FAILED",
                    "Development compilation failed. Check logs for errors."
                )
                
        except Exception as e:
            self.log_message(f"✗ Exception during compile: {str(e)}")
    
    def start_scheduler(self):
        """Start the build scheduler"""
        self.log_message("Alexander Nightly Build Scheduler Started")
        self.log_message(f"Log directory: {self.log_dir}")
        self.log_message("Scheduled tasks:")
        self.log_message("  - Daily Development Build: 2:00 AM")
        self.log_message("  - Weekly Shipping Build: Sunday 3:00 AM")
        self.log_message("  - Quick Compile Checks: Every 6 hours")
        
        # Schedule nightly development build at 2 AM
        schedule.every().day.at("02:00").do(self.nightly_development_build)
        
        # Schedule weekly shipping build on Sunday at 3 AM
        schedule.every().sunday.at("03:00").do(self.weekly_shipping_build)
        
        # Schedule quick compile checks every 6 hours
        schedule.every(6).hours.do(self.quick_compile_check)
        
        print("\nPress Ctrl+C to stop the scheduler\n")
        
        # Run scheduler loop
        try:
            while True:
                schedule.run_pending()
                time.sleep(60)  # Check every minute
        except KeyboardInterrupt:
            self.log_message("Scheduler stopped by user")
            print("\nScheduler stopped")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Alexander Nightly Build Scheduler')
    parser.add_argument('--email', action='store_true',
                       help='Enable email notifications')
    parser.add_argument('--test-now', action='store_true',
                       help='Run a build immediately (for testing)')
    
    args = parser.parse_args()
    
    scheduler = NightlyBuildScheduler(email_notifications=args.email)
    
    if args.test_now:
        print("Running test build immediately...")
        scheduler.nightly_development_build()
    else:
        scheduler.start_scheduler()

if __name__ == "__main__":
    main()
