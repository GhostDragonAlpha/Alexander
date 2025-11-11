#!/usr/bin/env python3
"""
Mission Generation and Validation Script for Alexander VR Space Simulation

This script creates, validates, and manages mission data assets for the game.
It ensures all missions meet the required format and can be properly loaded
by the mission system.

Usage:
    python create_missions.py --validate
    python create_missions.py --generate
    python create_missions.py --test-load
"""

import json
import os
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Any, Tuple
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class MissionValidator:
    """Validates mission data against the mission system requirements"""
    
    REQUIRED_FIELDS = [
        "MissionID", "MissionName", "MissionType", "Description", 
        "Objectives", "Rewards", "Location", "Difficulty"
    ]
    
    OBJECTIVE_REQUIRED_FIELDS = [
        "ID", "Type", "Description", "SuccessCondition"
    ]
    
    REWARD_REQUIRED_FIELDS = [
        "Credits", "XP"
    ]
    
    VALID_MISSION_TYPES = ["Combat", "Exploration", "Trading", "Escort", "SearchAndRescue"]
    VALID_DIFFICULTIES = ["Easy", "Medium", "Hard", "VeryHard"]
    VALID_OBJECTIVE_TYPES = [
        "Defend", "Eliminate", "Scan", "Collect", "Deliver", "Escort", 
        "Rescue", "Investigate", "Track", "Survive", "Navigate", "Trade", 
        "Land", "Explore", "Defend"
    ]
    
    def __init__(self, config_path: str = "Config/MissionConfig.json"):
        self.config_path = config_path
        self.config = self._load_config()
    
    def _load_config(self) -> Dict[str, Any]:
        """Load mission configuration"""
        try:
            with open(self.config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            logger.warning(f"Config file not found at {self.config_path}, using defaults")
            return {}
    
    def validate_mission(self, mission_data: Dict[str, Any]) -> Tuple[bool, List[str]]:
        """
        Validate a single mission data structure
        
        Returns:
            Tuple of (is_valid, list_of_errors)
        """
        errors = []
        
        # Check required top-level fields
        for field in self.REQUIRED_FIELDS:
            if field not in mission_data:
                errors.append(f"Missing required field: {field}")
        
        if errors:
            return False, errors
        
        # Validate MissionID format
        mission_id = mission_data.get("MissionID", "")
        if not mission_id or "_" not in mission_id:
            errors.append("Invalid MissionID format - should be 'Name_001' format")
        
        # Validate MissionType
        mission_type = mission_data.get("MissionType", "")
        if mission_type not in self.VALID_MISSION_TYPES:
            errors.append(f"Invalid MissionType: {mission_type}. Must be one of {self.VALID_MISSION_TYPES}")
        
        # Validate Difficulty
        difficulty = mission_data.get("Difficulty", "")
        if difficulty not in self.VALID_DIFFICULTIES:
            errors.append(f"Invalid Difficulty: {difficulty}. Must be one of {self.VALID_DIFFICULTIES}")
        
        # Validate Objectives
        objectives = mission_data.get("Objectives", [])
        if not objectives:
            errors.append("Mission must have at least one objective")
        else:
            for i, objective in enumerate(objectives):
                obj_errors = self._validate_objective(objective, i)
                errors.extend(obj_errors)
        
        # Validate Rewards
        rewards = mission_data.get("Rewards", {})
        reward_errors = self._validate_rewards(rewards)
        errors.extend(reward_errors)
        
        # Validate Location
        location = mission_data.get("Location", {})
        if not location.get("System"):
            errors.append("Location must have a System specified")
        
        # Validate LevelRequirement
        level_req = mission_data.get("LevelRequirement", 0)
        if not isinstance(level_req, int) or level_req < 0:
            errors.append("LevelRequirement must be a non-negative integer")
        
        # Validate TimeLimit
        time_limit = mission_data.get("TimeLimit", 0)
        if not isinstance(time_limit, int) or time_limit < 0:
            errors.append("TimeLimit must be a non-negative integer")
        
        return len(errors) == 0, errors
    
    def _validate_objective(self, objective: Dict[str, Any], index: int) -> List[str]:
        """Validate a single objective"""
        errors = []
        prefix = f"Objective[{index}]"
        
        for field in self.OBJECTIVE_REQUIRED_FIELDS:
            if field not in objective:
                errors.append(f"{prefix}: Missing required field: {field}")
        
        # Validate objective type
        obj_type = objective.get("Type", "")
        if obj_type not in self.VALID_OBJECTIVE_TYPES:
            errors.append(f"{prefix}: Invalid Type: {obj_type}. Must be one of {self.VALID_OBJECTIVE_TYPES}")
        
        # Validate progress tracking
        if "ProgressTrack" in objective and "RequiredValue" not in objective:
            errors.append(f"{prefix}: ProgressTrack specified but no RequiredValue")
        
        return errors
    
    def _validate_rewards(self, rewards: Dict[str, Any]) -> List[str]:
        """Validate rewards structure"""
        errors = []
        
        for field in self.REWARD_REQUIRED_FIELDS:
            if field not in rewards:
                errors.append(f"Rewards: Missing required field: {field}")
        
        # Validate Credits
        credits = rewards.get("Credits", 0)
        if not isinstance(credits, (int, float)) or credits < 0:
            errors.append("Rewards: Credits must be a non-negative number")
        
        # Validate XP
        xp = rewards.get("XP", 0)
        if not isinstance(xp, (int, float)) or xp < 0:
            errors.append("Rewards: XP must be a non-negative number")
        
        return errors


class MissionGenerator:
    """Generates and manages mission data assets"""
    
    def __init__(self, missions_dir: str = "Content/Missions"):
        self.missions_dir = Path(missions_dir)
        self.validator = MissionValidator()
        self.generated_missions = []
    
    def load_mission(self, file_path: Path) -> Tuple[bool, Dict[str, Any], List[str]]:
        """Load and validate a mission file"""
        try:
            with open(file_path, 'r') as f:
                mission_data = json.load(f)
            
            is_valid, errors = self.validator.validate_mission(mission_data)
            return is_valid, mission_data, errors
            
        except json.JSONDecodeError as e:
            return False, {}, [f"Invalid JSON in {file_path.name}: {str(e)}"]
        except Exception as e:
            return False, {}, [f"Error loading {file_path.name}: {str(e)}"]
    
    def load_all_missions(self) -> Tuple[List[Dict[str, Any]], List[str]]:
        """Load all mission files from the missions directory"""
        missions = []
        errors = []
        
        if not self.missions_dir.exists():
            errors.append(f"Missions directory not found: {self.missions_dir}")
            return missions, errors
        
        json_files = list(self.missions_dir.glob("Mission_*.json"))
        
        if not json_files:
            errors.append(f"No mission files found in {self.missions_dir}")
            return missions, errors
        
        for file_path in json_files:
            is_valid, mission_data, file_errors = self.load_mission(file_path)
            
            if file_errors:
                errors.extend([f"{file_path.name}: {err}" for err in file_errors])
            
            if is_valid:
                mission_data["_filename"] = file_path.name
                missions.append(mission_data)
        
        return missions, errors
    
    def validate_all_missions(self) -> Tuple[bool, List[str]]:
        """Validate all missions and return overall status"""
        missions, errors = self.load_all_missions()
        
        if not missions and not errors:
            errors.append("No missions found to validate")
            return False, errors
        
        # Group missions by type for analysis
        missions_by_type = {}
        for mission in missions:
            mtype = mission.get("MissionType", "Unknown")
            missions_by_type.setdefault(mtype, []).append(mission)
        
        # Check for balanced mission distribution
        if len(missions_by_type) < 3:
            errors.append(f"Low mission type variety: only {len(missions_by_type)} types found")
        
        # Check for duplicate MissionIDs
        mission_ids = [m.get("MissionID") for m in missions]
        duplicates = [mid for mid in mission_ids if mission_ids.count(mid) > 1]
        if duplicates:
            errors.append(f"Duplicate MissionIDs found: {set(duplicates)}")
        
        logger.info(f"Validated {len(missions)} missions across {len(missions_by_type)} types")
        for mtype, mlist in missions_by_type.items():
            logger.info(f"  - {mtype}: {len(mlist)} missions")
        
        return len(errors) == 0, errors
    
    def generate_mission_summary(self) -> Dict[str, Any]:
        """Generate a summary of all missions"""
        missions, errors = self.load_all_missions()
        
        summary = {
            "total_missions": len(missions),
            "validation_errors": errors,
            "missions_by_type": {},
            "difficulty_distribution": {},
            "level_requirements": {},
            "mission_givers": {}
        }
        
        for mission in missions:
            # Group by type
            mtype = mission.get("MissionType", "Unknown")
            summary["missions_by_type"].setdefault(mtype, 0)
            summary["missions_by_type"][mtype] += 1
            
            # Group by difficulty
            difficulty = mission.get("Difficulty", "Unknown")
            summary["difficulty_distribution"].setdefault(difficulty, 0)
            summary["difficulty_distribution"][difficulty] += 1
            
            # Track level requirements
            level_req = mission.get("LevelRequirement", 0)
            summary["level_requirements"].setdefault(level_req, 0)
            summary["level_requirements"][level_req] += 1
            
            # Track mission givers
            giver = mission.get("MissionGiver", "Unknown")
            summary["mission_givers"].setdefault(giver, 0)
            summary["mission_givers"][giver] += 1
        
        return summary
    
    def create_mission_registry(self) -> Dict[str, Any]:
        """Create a mission registry for game integration"""
        missions, errors = self.load_all_missions()
        
        registry = {
            "registry_version": "1.0",
            "total_missions": len(missions),
            "validation_status": "valid" if not errors else "has_errors",
            "validation_errors": errors,
            "missions": []
        }
        
        for mission in missions:
            registry_entry = {
                "mission_id": mission.get("MissionID"),
                "mission_name": mission.get("MissionName"),
                "mission_type": mission.get("MissionType"),
                "difficulty": mission.get("Difficulty"),
                "level_requirement": mission.get("LevelRequirement"),
                "time_limit": mission.get("TimeLimit"),
                "mission_giver": mission.get("MissionGiver"),
                "prerequisites": mission.get("Prerequisites", []),
                "location": mission.get("Location", {}).get("System"),
                "filename": mission.get("_filename")
            }
            registry["missions"].append(registry_entry)
        
        return registry


def main():
    parser = argparse.ArgumentParser(description="Mission management tool for Alexander VR")
    parser.add_argument("--validate", action="store_true", help="Validate all mission files")
    parser.add_argument("--generate", action="store_true", help="Generate mission registry")
    parser.add_argument("--test-load", action="store_true", help="Test loading all missions")
    parser.add_argument("--summary", action="store_true", help="Generate mission summary")
    parser.add_argument("--output", type=str, default="Content/Data/Missions/mission_registry.json", help="Output file for registry")
    
    args = parser.parse_args()
    
    generator = MissionGenerator()
    
    if args.validate or not any([args.generate, args.test_load, args.summary]):
        logger.info("Validating all mission files...")
        is_valid, errors = generator.validate_all_missions()
        
        if is_valid:
            logger.info("✅ All missions validated successfully!")
        else:
            logger.error("❌ Mission validation failed:")
            for error in errors:
                logger.error(f"  - {error}")
            sys.exit(1)
    
    if args.test_load:
        logger.info("Testing mission loading...")
        missions, errors = generator.load_all_missions()
        
        if errors:
            logger.error("❌ Errors loading missions:")
            for error in errors:
                logger.error(f"  - {error}")
            sys.exit(1)
        else:
            logger.info(f"✅ Successfully loaded {len(missions)} missions")
    
    if args.summary:
        logger.info("Generating mission summary...")
        summary = generator.generate_mission_summary()
        
        print("\n" + "="*60)
        print("MISSION SUMMARY")
        print("="*60)
        print(f"Total Missions: {summary['total_missions']}")
        print(f"Validation Errors: {len(summary['validation_errors'])}")
        print("\nMissions by Type:")
        for mtype, count in summary['missions_by_type'].items():
            print(f"  - {mtype}: {count}")
        print("\nDifficulty Distribution:")
        for difficulty, count in summary['difficulty_distribution'].items():
            print(f"  - {difficulty}: {count}")
        print("\nMission Givers:")
        for giver, count in summary['mission_givers'].items():
            print(f"  - {giver}: {count}")
        print("="*60 + "\n")
    
    if args.generate:
        logger.info(f"Generating mission registry to {args.output}...")
        registry = generator.create_mission_registry()
        
        # Ensure output directory exists
        output_path = Path(args.output)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'w') as f:
            json.dump(registry, f, indent=2)
        
        logger.info(f"✅ Mission registry generated: {args.output}")


if __name__ == "__main__":
    main()