#!/usr/bin/env python3
"""
Data Management System
Manages screenshot storage, data archiving, retention policies, and backups
"""

import json
import time
import logging
import shutil
import zipfile
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime, timedelta
import sqlite3
import threading

# Configuration
PLAYTESTING_DATA_DIR = Path("playtesting_data")
SCREENSHOTS_DIR = Path("screenshots_visual_playtest")
ARCHIVE_DIR = Path("playtesting_data/archive")
BACKUP_DIR = Path("playtesting_data/backups")
CONFIG_DIR = Path("automation_config")
LOG_FILE = CONFIG_DIR / "data_manager.log"

class DataManager:
    """Manages playtesting data storage, archiving, and cleanup"""
    
    def __init__(self, config_path: str = "playtesting_config.json"):
        self.config = self.load_config(config_path)
        self.running = False
        self.cleanup_thread: Optional[threading.Thread] = None
        
        # Setup directories
        PLAYTESTING_DATA_DIR.mkdir(parents=True, exist_ok=True)
        SCREENSHOTS_DIR.mkdir(exist_ok=True)
        ARCHIVE_DIR.mkdir(parents=True, exist_ok=True)
        BACKUP_DIR.mkdir(parents=True, exist_ok=True)
        CONFIG_DIR.mkdir(exist_ok=True)
        
        # Setup logging
        self.setup_logging()
        
        # Configuration
        self.retention_days = self.config.get("data_management", {}).get("data_retention_days", 30)
        self.compress_old_data = self.config.get("data_management", {}).get("compress_old_data", True)
        self.cleanup_interval_hours = self.config.get("data_management", {}).get("cleanup_interval_hours", 24)
        
        self.log("Data manager initialized")
        
    def setup_logging(self):
        """Setup logging configuration"""
        logging.basicConfig(
            filename=LOG_FILE,
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        self.logger = logging.getLogger(__name__)
        
    def log(self, message: str, level: str = "INFO"):
        """Log message"""
        if level == "INFO":
            self.logger.info(message)
        elif level == "WARNING":
            self.logger.warning(message)
        elif level == "ERROR":
            self.logger.error(message)
        elif level == "DEBUG":
            self.logger.debug(message)
            
        print(f"[DataManager] {level}: {message}")
        
    def load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            self.log(f"Error loading config: {e}, using defaults", "WARNING")
            return {
                "data_management": {
                    "data_retention_days": 30,
                    "compress_old_data": True,
                    "cleanup_interval_hours": 24
                }
            }
            
    def get_directory_size(self, directory: Path) -> int:
        """Get total size of directory in bytes"""
        try:
            total_size = 0
            for item in directory.rglob('*'):
                if item.is_file():
                    total_size += item.stat().st_size
            return total_size
        except Exception as e:
            self.log(f"Error calculating directory size: {e}", "WARNING")
            return 0
            
    def format_size(self, size_bytes: int) -> str:
        """Format bytes to human readable string"""
        for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
            if size_bytes < 1024.0:
                return f"{size_bytes:.1f} {unit}"
            size_bytes /= 1024.0
        return f"{size_bytes:.1f} PB"
        
    def archive_old_sessions(self, days_old: int = 7):
        """Archive sessions older than specified days"""
        try:
            cutoff_date = datetime.now() - timedelta(days=days_old)
            archived_count = 0
            
            # Archive performance data
            performance_dir = PLAYTESTING_DATA_DIR / "performance"
            if performance_dir.exists():
                for session_dir in performance_dir.iterdir():
                    if session_dir.is_dir():
                        try:
                            dir_time = datetime.fromtimestamp(session_dir.stat().st_mtime)
                            if dir_time < cutoff_date:
                                self.archive_session(session_dir)
                                archived_count += 1
                        except Exception as e:
                            self.log(f"Error checking session directory {session_dir}: {e}", "WARNING")
                            
            # Archive screenshots
            screenshot_files = list(SCREENSHOTS_DIR.glob("screenshot_*.jpg"))
            for screenshot_file in screenshot_files:
                try:
                    file_time = datetime.fromtimestamp(screenshot_file.stat().st_mtime)
                    if file_time < cutoff_date:
                        self.archive_file(screenshot_file, "screenshots")
                        archived_count += 1
                except Exception as e:
                    self.log(f"Error checking screenshot file {screenshot_file}: {e}", "WARNING")
                    
            if archived_count > 0:
                self.log(f"Archived {archived_count} old files/directories")
                
        except Exception as e:
            self.log(f"Error archiving old sessions: {e}", "ERROR")
            
    def archive_session(self, session_dir: Path):
        """Archive a session directory"""
        try:
            # Create archive filename
            archive_name = f"{session_dir.name}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.zip"
            archive_path = ARCHIVE_DIR / archive_name
            
            # Create zip archive
            with zipfile.ZipFile(archive_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
                for file_path in session_dir.rglob('*'):
                    if file_path.is_file():
                        arcname = file_path.relative_to(session_dir)
                        zipf.write(file_path, arcname)
                        
            # Remove original directory
            shutil.rmtree(session_dir)
            
            self.log(f"Session archived: {archive_path}")
            
        except Exception as e:
            self.log(f"Error archiving session {session_dir}: {e}", "ERROR")
            
    def archive_file(self, file_path: Path, category: str):
        """Archive a single file"""
        try:
            # Create category directory in archive
            category_dir = ARCHIVE_DIR / category
            category_dir.mkdir(exist_ok=True)
            
            # Move file to archive
            archive_path = category_dir / file_path.name
            shutil.move(str(file_path), str(archive_path))
            
            self.log(f"File archived: {archive_path}")
            
        except Exception as e:
            self.log(f"Error archiving file {file_path}: {e}", "ERROR")
            
    def cleanup_old_data(self, days_to_keep: int = None):
        """Clean up data older than retention period"""
        if days_to_keep is None:
            days_to_keep = self.retention_days
            
        try:
            self.log(f"Starting cleanup of data older than {days_to_keep} days")
            
            cutoff_date = datetime.now() - timedelta(days=days_to_keep)
            deleted_count = 0
            total_space_freed = 0
            
            # Clean up performance data
            performance_dir = PLAYTESTING_DATA_DIR / "performance"
            if performance_dir.exists():
                for session_dir in performance_dir.iterdir():
                    if session_dir.is_dir():
                        try:
                            dir_time = datetime.fromtimestamp(session_dir.stat().st_mtime)
                            if dir_time < cutoff_date:
                                size = self.get_directory_size(session_dir)
                                shutil.rmtree(session_dir)
                                total_space_freed += size
                                deleted_count += 1
                        except Exception as e:
                            self.log(f"Error deleting session directory {session_dir}: {e}", "WARNING")
                            
            # Clean up screenshots
            screenshot_files = list(SCREENSHOTS_DIR.glob("screenshot_*.jpg"))
            for screenshot_file in screenshot_files:
                try:
                    file_time = datetime.fromtimestamp(screenshot_file.stat().st_mtime)
                    if file_time < cutoff_date:
                        size = screenshot_file.stat().st_size
                        screenshot_file.unlink()
                        total_space_freed += size
                        deleted_count += 1
                except Exception as e:
                    self.log(f"Error deleting screenshot file {screenshot_file}: {e}", "WARNING")
                    
            # Clean up old archives if they exceed retention period significantly
            archive_retention_days = days_to_keep * 2  # Keep archives longer
            archive_cutoff = datetime.now() - timedelta(days=archive_retention_days)
            
            if ARCHIVE_DIR.exists():
                for archive_file in ARCHIVE_DIR.rglob("*.zip"):
                    try:
                        file_time = datetime.fromtimestamp(archive_file.stat().st_mtime)
                        if file_time < archive_cutoff:
                            size = archive_file.stat().st_size
                            archive_file.unlink()
                            total_space_freed += size
                            deleted_count += 1
                    except Exception as e:
                        self.log(f"Error deleting archive file {archive_file}: {e}", "WARNING")
                        
            self.log(f"Cleanup completed: {deleted_count} items deleted, {self.format_size(total_space_freed)} freed")
            
        except Exception as e:
            self.log(f"Error during cleanup: {e}", "ERROR")
            
    def compress_old_archives(self, days_old: int = 14):
        """Compress archives older than specified days"""
        if not self.compress_old_data:
            return
            
        try:
            cutoff_date = datetime.now() - timedelta(days=days_old)
            compressed_count = 0
            
            for archive_file in ARCHIVE_DIR.rglob("*.zip"):
                try:
                    file_time = datetime.fromtimestamp(archive_file.stat().st_mtime)
                    if file_time < cutoff_date and archive_file.stat().st_size > 10 * 1024 * 1024:  # > 10MB
                        # Recompress with higher compression
                        temp_dir = ARCHIVE_DIR / "temp_compression"
                        temp_dir.mkdir(exist_ok=True)
                        
                        # Extract and recompress
                        with zipfile.ZipFile(archive_file, 'r') as zipf:
                            zipf.extractall(temp_dir)
                            
                        # Create new compressed archive
                        new_archive = archive_file.with_name(f"{archive_file.stem}_recompressed.zip")
                        with zipfile.ZipFile(new_archive, 'w', zipfile.ZIP_BZIP2) as new_zipf:
                            for file_path in temp_dir.rglob('*'):
                                if file_path.is_file():
                                    arcname = file_path.relative_to(temp_dir)
                                    new_zipf.write(file_path, arcname)
                                    
                        # Replace old archive if new one is smaller
                        if new_archive.stat().st_size < archive_file.stat().st_size:
                            archive_file.unlink()
                            new_archive.rename(archive_file)
                            compressed_count += 1
                            
                        # Cleanup temp directory
                        shutil.rmtree(temp_dir)
                        
                except Exception as e:
                    self.log(f"Error compressing archive {archive_file}: {e}", "WARNING")
                    
            if compressed_count > 0:
                self.log(f"Compressed {compressed_count} old archives")
                
        except Exception as e:
            self.log(f"Error compressing old archives: {e}", "ERROR")
            
    def backup_critical_findings(self):
        """Backup critical findings and important data"""
        try:
            backup_timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup_name = f"critical_findings_backup_{backup_timestamp}.zip"
            backup_path = BACKUP_DIR / backup_name
            
            with zipfile.ZipFile(backup_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
                # Backup issue database
                issues_db = Path("playtesting_data/issues.db")
                if issues_db.exists():
                    zipf.write(issues_db, "issues.db")
                    
                # Backup recent performance reports
                performance_dir = PLAYTESTING_DATA_DIR / "performance"
                if performance_dir.exists():
                    for session_dir in performance_dir.iterdir():
                        if session_dir.is_dir():
                            # Add recent sessions (last 7 days)
                            dir_time = datetime.fromtimestamp(session_dir.stat().st_mtime)
                            if datetime.now() - dir_time < timedelta(days=7):
                                for file_path in session_dir.rglob('*'):
                                    if file_path.is_file():
                                        arcname = f"performance/{session_dir.name}/{file_path.name}"
                                        zipf.write(file_path, arcname)
                                        
                # Backup configuration
                config_files = [
                    "playtesting_config.json",
                    "automation_config/screenshot_config.json"
                ]
                
                for config_file in config_files:
                    config_path = Path(config_file)
                    if config_path.exists():
                        zipf.write(config_path, f"config/{config_path.name}")
                        
            self.log(f"Critical findings backed up: {backup_path}")
            
        except Exception as e:
            self.log(f"Error backing up critical findings: {e}", "ERROR")
            
    def get_storage_stats(self) -> Dict[str, Any]:
        """Get storage statistics"""
        try:
            stats = {
                "timestamp": datetime.now().isoformat(),
                "directories": {}
            }
            
            # Calculate sizes for key directories
            directories = {
                "screenshots": SCREENSHOTS_DIR,
                "performance_data": PLAYTESTING_DATA_DIR / "performance",
                "archives": ARCHIVE_DIR,
                "backups": BACKUP_DIR,
                "issues": Path("playtesting_data")
            }
            
            total_size = 0
            
            for name, directory in directories.items():
                if directory.exists():
                    size = self.get_directory_size(directory)
                    stats["directories"][name] = {
                        "size_bytes": size,
                        "size_readable": self.format_size(size),
                        "file_count": len(list(directory.rglob('*'))) if directory.is_dir() else 0
                    }
                    total_size += size
                    
            stats["total_size_bytes"] = total_size
            stats["total_size_readable"] = self.format_size(total_size)
            
            return stats
            
        except Exception as e:
            self.log(f"Error getting storage stats: {e}", "ERROR")
            return {"error": str(e)}
            
    def cleanup_loop(self):
        """Background cleanup loop"""
        self.log("Starting data management cleanup loop...")
        
        while self.running:
            try:
                # Archive old sessions (older than 7 days)
                self.archive_old_sessions(days_old=7)
                
                # Clean up old data (older than retention period)
                self.cleanup_old_data()
                
                # Compress old archives
                self.compress_old_archives()
                
                # Backup critical findings (daily)
                if datetime.now().hour == 2:  # 2 AM
                    self.backup_critical_findings()
                    
                # Sleep until next cleanup (check every hour)
                for _ in range(3600):  # 3600 seconds = 1 hour
                    if not self.running:
                        break
                    time.sleep(1)
                    
            except Exception as e:
                self.log(f"Error in cleanup loop: {e}", "ERROR")
                time.sleep(3600)  # Wait an hour before retrying
                
        self.log("Data management cleanup loop stopped")
        
    def start(self):
        """Start background cleanup"""
        if self.running:
            self.log("Cleanup already running", "WARNING")
            return
            
        self.running = True
        self.cleanup_thread = threading.Thread(target=self.cleanup_loop, daemon=True)
        self.cleanup_thread.start()
        
        self.log("Data management started")
        
    def stop(self):
        """Stop background cleanup"""
        if not self.running:
            self.log("Cleanup not running", "WARNING")
            return
            
        self.running = False
        
        if self.cleanup_thread:
            self.cleanup_thread.join(timeout=5)
            
        self.log("Data management stopped")
        
    def export_data(self, export_type: str = "all", start_date: datetime = None, 
                   end_date: datetime = None) -> Optional[Path]:
        """Export data for analysis"""
        try:
            if start_date is None:
                start_date = datetime.now() - timedelta(days=7)
            if end_date is None:
                end_date = datetime.now()
                
            export_timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            export_name = f"playtesting_export_{export_type}_{export_timestamp}.zip"
            export_path = PLAYTESTING_DATA_DIR / "exports" / export_name
            
            # Create export directory
            export_path.parent.mkdir(parents=True, exist_ok=True)
            
            with zipfile.ZipFile(export_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
                if export_type in ["all", "performance"]:
                    # Export performance data
                    performance_dir = PLAYTESTING_DATA_DIR / "performance"
                    if performance_dir.exists():
                        for session_dir in performance_dir.iterdir():
                            if session_dir.is_dir():
                                session_time = datetime.fromtimestamp(session_dir.stat().st_mtime)
                                if start_date <= session_time <= end_date:
                                    for file_path in session_dir.rglob('*'):
                                        if file_path.is_file():
                                            arcname = f"performance/{session_dir.name}/{file_path.name}"
                                            zipf.write(file_path, arcname)
                                            
                if export_type in ["all", "issues"]:
                    # Export issues database
                    issues_db = Path("playtesting_data/issues.db")
                    if issues_db.exists():
                        zipf.write(issues_db, "issues.db")
                        
                if export_type in ["all", "screenshots"]:
                    # Export screenshots
                    if SCREENSHOTS_DIR.exists():
                        screenshot_files = list(SCREENSHOTS_DIR.glob("screenshot_*.jpg"))
                        for screenshot_file in screenshot_files:
                            file_time = datetime.fromtimestamp(screenshot_file.stat().st_mtime)
                            if start_date <= file_time <= end_date:
                                arcname = f"screenshots/{screenshot_file.name}"
                                zipf.write(screenshot_file, arcname)
                                
            self.log(f"Data exported: {export_path}")
            return export_path
            
        except Exception as e:
            self.log(f"Error exporting data: {e}", "ERROR")
            return None

def main():
    """Main function for standalone usage"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Data Management System")
    parser.add_argument("--action", type=str, choices=["stats", "cleanup", "backup", "export"],
                       required=True, help="Action to perform")
    parser.add_argument("--config", type=str, default="playtesting_config.json",
                       help="Configuration file path")
    parser.add_argument("--export-type", type=str, choices=["all", "performance", "issues", "screenshots"],
                       default="all", help="Type of data to export")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("DATA MANAGEMENT SYSTEM")
    print("=" * 70)
    
    # Create data manager
    manager = DataManager(args.config)
    
    if args.action == "stats":
        stats = manager.get_storage_stats()
        print("\n" + "=" * 70)
        print("STORAGE STATISTICS")
        print("=" * 70)
        print(json.dumps(stats, indent=2))
        print("=" * 70)
        
    elif args.action == "cleanup":
        print("\nStarting cleanup...")
        manager.cleanup_old_data()
        print("Cleanup completed")
        
    elif args.action == "backup":
        print("\nCreating backup...")
        manager.backup_critical_findings()
        print("Backup completed")
        
    elif args.action == "export":
        print(f"\nExporting {args.export_type} data...")
        export_path = manager.export_data(export_type=args.export_type)
        if export_path:
            print(f"Export completed: {export_path}")
        else:
            print("Export failed")

if __name__ == "__main__":
    main()