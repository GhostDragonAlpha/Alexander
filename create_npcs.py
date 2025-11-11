#!/usr/bin/env python3
"""
NPC Generation and Management System for Alexander VR Space Simulation

This script provides:
- NPC generation from archetype templates
- NPC data validation
- NPC management utilities
- Integration with existing game systems
"""

import json
import os
import random
import uuid
from datetime import datetime
from typing import Dict, List, Optional, Tuple, Any
from pathlib import Path

class NPCGenerator:
    """Main NPC generation and management class"""
    
    def __init__(self, config_path: str = "Config/NPCConfig.json", npcs_path: str = "Content/NPCs"):
        self.config_path = Path(config_path)
        self.npcs_path = Path(npcs_path)
        self.config = self._load_config()
        self.generated_npcs = []
        self.validation_errors = []
        
    def _load_config(self) -> Dict[str, Any]:
        """Load NPC configuration from JSON file"""
        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"Error: Config file not found at {self.config_path}")
            return {}
        except json.JSONDecodeError as e:
            print(f"Error: Invalid JSON in config file: {e}")
            return {}
    
    def _load_npc_template(self, archetype: str) -> Optional[Dict[str, Any]]:
        """Load NPC template for given archetype"""
        template_path = self.npcs_path / f"NPC_{archetype}.json"
        try:
            with open(template_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"Warning: Template not found for archetype {archetype}")
            return None
        except json.JSONDecodeError as e:
            print(f"Error: Invalid JSON in template {archetype}: {e}")
            return None
    
    def generate_npc(self, archetype: str, location: Optional[Dict[str, str]] = None) -> Optional[Dict[str, Any]]:
        """
        Generate a new NPC based on archetype template
        
        Args:
            archetype: NPC archetype name
            location: Optional location override
            
        Returns:
            Generated NPC data or None if failed
        """
        template = self._load_npc_template(archetype)
        if not template:
            return None
        
        # Generate unique NPC ID
        npc_id = f"{archetype}_{str(uuid.uuid4())[:8]}"
        
        # Create NPC based on template
        npc = template.copy()
        npc["NPCID"] = npc_id
        npc["Generated"] = datetime.now().isoformat()
        
        # Randomize some properties for variety
        if "Personality" in npc:
            traits = ["Friendly", "Neutral", "Suspicious", "Professional", "Enthusiastic", "Gruff", "Mysterious"]
            npc["Personality"]["Trait"] = random.choice(traits)
        
        # Randomize location if not specified
        if location and "Location" in npc:
            npc["Location"].update(location)
        elif "Location" in npc:
            systems = ["Sol", "AlphaCentauri", "Vega", "Sirius", "Kepler", "Proxima", "Omega", "Frontier"]
            stations = ["Station_Alpha", "Commerce_Hub", "Research_Outpost", "Mining_Facility", "Trade_Post"]
            npc["Location"]["System"] = random.choice(systems)
            npc["Location"]["Station"] = random.choice(stations)
        
        # Randomize appearance variations
        if "Appearance" in npc:
            models = ["Human_Male_01", "Human_Female_01", "Human_Male_02", "Human_Female_02", "Alien_01"]
            npc["Appearance"]["Model"] = random.choice(models)
        
        # Add random inventory variations
        if "Shop" in npc and "Sells" in npc["Shop"]:
            # Add some random items from common pool
            common_items = self.config.get("ShopInventory", {}).get("CommonItems", {})
            for category in npc["Shop"]["Sells"]:
                if category in common_items:
                    # Randomly add 1-3 items from each category
                    available_items = common_items[category]
                    num_items = random.randint(1, min(3, len(available_items)))
                    selected_items = random.sample(available_items, num_items)
                    npc["Shop"][f"{category}_Inventory"] = selected_items
        
        return npc
    
    def generate_npcs_batch(self, count: int, archetypes: Optional[List[str]] = None) -> List[Dict[str, Any]]:
        """
        Generate multiple NPCs in batch
        
        Args:
            count: Number of NPCs to generate
            archetypes: List of archetypes to choose from (random if None)
            
        Returns:
            List of generated NPCs
        """
        if not archetypes:
            archetypes = list(self.config.get("ArchetypeDefinitions", {}).keys())
        
        if not archetypes:
            print("Error: No archetypes available")
            return []
        
        generated = []
        for i in range(count):
            archetype = random.choice(archetypes)
            npc = self.generate_npc(archetype)
            if npc:
                generated.append(npc)
                print(f"Generated NPC {i+1}/{count}: {npc['Name']} ({archetype})")
        
        self.generated_npcs.extend(generated)
        return generated
    
    def validate_npc(self, npc_data: Dict[str, Any]) -> Tuple[bool, List[str]]:
        """
        Validate NPC data structure and required fields
        
        Args:
            npc_data: NPC data to validate
            
        Returns:
            Tuple of (is_valid, error_messages)
        """
        errors = []
        required_fields = ["NPCID", "Name", "Archetype", "Location", "Personality"]
        
        # Check required fields
        for field in required_fields:
            if field not in npc_data:
                errors.append(f"Missing required field: {field}")
        
        # Validate archetype exists
        archetype = npc_data.get("Archetype")
        if archetype and archetype not in self.config.get("ArchetypeDefinitions", {}):
            errors.append(f"Unknown archetype: {archetype}")
        
        # Validate location structure
        if "Location" in npc_data:
            location = npc_data["Location"]
            if not isinstance(location, dict):
                errors.append("Location must be a dictionary")
            elif "System" not in location or "Station" not in location:
                errors.append("Location must contain 'System' and 'Station'")
        
        # Validate shop structure if present
        if "Shop" in npc_data:
            shop = npc_data["Shop"]
            if not isinstance(shop, dict):
                errors.append("Shop must be a dictionary")
            else:
                if "Sells" in shop and not isinstance(shop["Sells"], list):
                    errors.append("Shop.Sells must be a list")
                if "Buys" in shop and not isinstance(shop["Buys"], list):
                    errors.append("Shop.Buys must be a list")
        
        # Validate missions structure if present
        if "Missions" in npc_data:
            missions = npc_data["Missions"]
            if not isinstance(missions, dict):
                errors.append("Missions must be a dictionary")
            else:
                if "Offers" in missions and not isinstance(missions["Offers"], list):
                    errors.append("Missions.Offers must be a list")
                if "Weight" in missions and not isinstance(missions["Weight"], (int, float)):
                    errors.append("Missions.Weight must be a number")
        
        # Validate relationships structure if present
        if "Relationships" in npc_data:
            relationships = npc_data["Relationships"]
            if not isinstance(relationships, dict):
                errors.append("Relationships must be a dictionary")
            else:
                if "InitialStanding" in relationships and not isinstance(relationships["InitialStanding"], (int, float)):
                    errors.append("Relationships.InitialStanding must be a number")
        
        return len(errors) == 0, errors
    
    def validate_all_npcs(self, npcs_path: Optional[str] = None) -> Tuple[int, int, List[str]]:
        """
        Validate all NPC files in the specified directory
        
        Args:
            npcs_path: Path to NPC files (uses default if None)
            
        Returns:
            Tuple of (valid_count, invalid_count, error_messages)
        """
        if not npcs_path:
            npcs_path = self.npcs_path
        
        npc_files = list(Path(npcs_path).glob("NPC_*.json"))
        valid_count = 0
        invalid_count = 0
        all_errors = []
        
        print(f"Validating {len(npc_files)} NPC files...")
        
        for npc_file in npc_files:
            try:
                with open(npc_file, 'r', encoding='utf-8') as f:
                    npc_data = json.load(f)
                
                is_valid, errors = self.validate_npc(npc_data)
                if is_valid:
                    valid_count += 1
                    print(f"[OK] {npc_file.name} - Valid")
                else:
                    invalid_count += 1
                    print(f"[FAIL] {npc_file.name} - Invalid:")
                    for error in errors:
                        print(f"  - {error}")
                    all_errors.extend([f"{npc_file.name}: {error}" for error in errors])
            
            except Exception as e:
                invalid_count += 1
                error_msg = f"Error reading {npc_file.name}: {str(e)}"
                print(f"[ERROR] {error_msg}")
                all_errors.append(error_msg)
        
        print(f"\nValidation Summary: {valid_count} valid, {invalid_count} invalid")
        return valid_count, invalid_count, all_errors
    
    def save_npc(self, npc_data: Dict[str, Any], filename: Optional[str] = None) -> bool:
        """
        Save NPC data to file
        
        Args:
            npc_data: NPC data to save
            filename: Optional filename (auto-generated if None)
            
        Returns:
            True if successful, False otherwise
        """
        if not filename:
            archetype = npc_data.get("Archetype", "Unknown")
            npc_id = npc_data.get("NPCID", str(uuid.uuid4())[:8])
            filename = f"NPC_{archetype}_{npc_id}.json"
        
        filepath = self.npcs_path / filename
        
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                json.dump(npc_data, f, indent=2, ensure_ascii=False)
            print(f"Saved NPC to {filepath}")
            return True
        except Exception as e:
            print(f"Error saving NPC: {e}")
            return False
    
    def export_npcs_batch(self, npcs: List[Dict[str, Any]], prefix: str = "batch") -> int:
        """
        Export multiple NPCs to files
        
        Args:
            npcs: List of NPCs to export
            prefix: Filename prefix
            
        Returns:
            Number of successfully exported NPCs
        """
        success_count = 0
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        for i, npc in enumerate(npcs):
            filename = f"{prefix}_{timestamp}_{i+1:03d}.json"
            if self.save_npc(npc, filename):
                success_count += 1
        
        print(f"Exported {success_count}/{len(npcs)} NPCs")
        return success_count
    
    def get_npc_statistics(self) -> Dict[str, Any]:
        """Get statistics about NPCs and archetypes"""
        archetype_counts = {}
        faction_counts = {}
        location_counts = {}
        
        # Count from generated NPCs
        for npc in self.generated_npcs:
            archetype = npc.get("Archetype", "Unknown")
            faction = npc.get("Faction", "Unknown")
            location = npc.get("Location", {})
            system = location.get("System", "Unknown")
            
            archetype_counts[archetype] = archetype_counts.get(archetype, 0) + 1
            faction_counts[faction] = faction_counts.get(faction, 0) + 1
            location_counts[system] = location_counts.get(system, 0) + 1
        
        # Count from template files
        template_files = list(self.npcs_path.glob("NPC_*.json"))
        
        return {
            "generated_npcs": len(self.generated_npcs),
            "template_files": len(template_files),
            "archetype_distribution": archetype_counts,
            "faction_distribution": faction_counts,
            "location_distribution": location_counts,
            "available_archetypes": list(self.config.get("ArchetypeDefinitions", {}).keys())
        }
    
    def generate_test_scenario(self, scenario_type: str = "balanced") -> List[Dict[str, Any]]:
        """
        Generate a test scenario with specific NPC distribution
        
        Args:
            scenario_type: Type of scenario (balanced, combat_focused, trade_focused, etc.)
            
        Returns:
            List of NPCs for the scenario
        """
        scenarios = {
            "balanced": {
                "counts": {
                    "ShipDealer": 1,
                    "EquipmentVendor": 1,
                    "CommodityTrader": 2,
                    "BountyHunter": 1,
                    "MissionCommander": 1,
                    "ShipMechanic": 1,
                    "RefuelTechnician": 1,
                    "MedicalOfficer": 1,
                    "InformationBroker": 1
                }
            },
            "combat_focused": {
                "counts": {
                    "BountyHunter": 2,
                    "MissionCommander": 2,
                    "SecurityChief": 1,
                    "EquipmentVendor": 2,
                    "ShipMechanic": 1
                }
            },
            "trade_focused": {
                "counts": {
                    "CommodityTrader": 3,
                    "ShipDealer": 2,
                    "EquipmentVendor": 1,
                    "CustomsOfficer": 1,
                    "InformationBroker": 1,
                    "RefuelTechnician": 1
                }
            },
            "exploration_focused": {
                "counts": {
                    "ScienceOfficer": 2,
                    "Explorer": 2,
                    "InformationBroker": 1,
                    "RescueCoordinator": 1,
                    "RefuelTechnician": 1,
                    "ShipMechanic": 1
                }
            }
        }
        
        scenario = scenarios.get(scenario_type, scenarios["balanced"])
        generated_npcs = []
        
        for archetype, count in scenario["counts"].items():
            for i in range(count):
                npc = self.generate_npc(archetype)
                if npc:
                    generated_npcs.append(npc)
        
        print(f"Generated {len(generated_npcs)} NPCs for {scenario_type} scenario")
        return generated_npcs
    
    def integrate_with_mission_system(self, npc_data: Dict[str, Any]) -> List[Dict[str, Any]]:
        """
        Generate mission offers based on NPC archetype and standing
        
        Args:
            npc_data: NPC data to generate missions for
            
        Returns:
            List of available missions
        """
        archetype = npc_data.get("Archetype")
        if not archetype:
            return []
        
        archetype_config = self.config.get("ArchetypeDefinitions", {}).get(archetype, {})
        mission_types = archetype_config.get("MissionTypes", [])
        
        missions = []
        for mission_type in mission_types:
            mission_template = self.config.get("MissionTemplates", {}).get(mission_type.split("_")[0], {})
            if mission_template:
                mission = {
                    "Type": mission_type,
                    "Difficulty": mission_template.get("Difficulty", "Medium"),
                    "Duration": mission_template.get("Duration", "Medium"),
                    "Rewards": mission_template.get("Rewards", ["Credits"]),
                    "OfferedBy": npc_data.get("NPCID"),
                    "ExpiryTime": 3600  # 1 hour
                }
                missions.append(mission)
        
        return missions
    
    def integrate_with_trade_system(self, npc_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Generate shop inventory based on NPC archetype and standing
        
        Args:
            npc_data: NPC data to generate shop for
            
        Returns:
            Shop inventory data
        """
        archetype = npc_data.get("Archetype")
        if not archetype:
            return {}
        
        archetype_config = self.config.get("ArchetypeDefinitions", {}).get(archetype, {})
        shop_categories = archetype_config.get("ShopCategories", [])
        
        shop_inventory = {
            "NPCID": npc_data.get("NPCID"),
            "Categories": {},
            "PriceModifier": npc_data.get("Shop", {}).get("PriceModifier", 1.0)
        }
        
        shop_items = self.config.get("ShopInventory", {})
        
        for category in shop_categories:
            if category in shop_items.get("CommonItems", {}):
                items = shop_items["CommonItems"][category]
                # Randomly select items for this shop
                num_items = random.randint(2, min(5, len(items)))
                selected_items = random.sample(items, num_items)
                shop_inventory["Categories"][category] = selected_items
        
        return shop_inventory


def main():
    """Main function for command-line usage"""
    import argparse
    
    parser = argparse.ArgumentParser(description="NPC Generation and Management System")
    parser.add_argument("--generate", type=int, help="Generate specified number of NPCs")
    parser.add_argument("--archetype", help="Specific archetype to generate")
    parser.add_argument("--validate", action="store_true", help="Validate all NPC files")
    parser.add_argument("--scenario", help="Generate test scenario (balanced, combat_focused, trade_focused, exploration_focused)")
    parser.add_argument("--export", action="store_true", help="Export generated NPCs to files")
    parser.add_argument("--stats", action="store_true", help="Show NPC statistics")
    parser.add_argument("--test-integration", action="store_true", help="Test integration with other systems")
    
    args = parser.parse_args()
    
    # Initialize generator
    generator = NPCGenerator()
    
    # Handle commands
    if args.validate:
        valid, invalid, errors = generator.validate_all_npcs()
        print(f"\nValidation complete: {valid} valid, {invalid} invalid")
        if errors:
            print("\nErrors found:")
            for error in errors:
                print(f"  - {error}")
    
    if args.generate:
        archetypes = [args.archetype] if args.archetype else None
        npcs = generator.generate_npcs_batch(args.generate, archetypes)
        
        if args.export:
            generator.export_npcs_batch(npcs, "generated")
        
        if args.test_integration:
            print("\nTesting system integration:")
            for npc in npcs[:3]:  # Test first 3 NPCs
                missions = generator.integrate_with_mission_system(npc)
                shop = generator.integrate_with_trade_system(npc)
                print(f"  {npc['Name']}: {len(missions)} missions, {len(shop.get('Categories', {}))} shop categories")
    
    if args.scenario:
        scenario_npcs = generator.generate_test_scenario(args.scenario)
        
        if args.export:
            generator.export_npcs_batch(scenario_npcs, f"scenario_{args.scenario}")
    
    if args.stats:
        stats = generator.get_npc_statistics()
        print("\nNPC Statistics:")
        print(f"Generated NPCs: {stats['generated_npcs']}")
        print(f"Template Files: {stats['template_files']}")
        print(f"Available Archetypes: {len(stats['available_archetypes'])}")
        print("\nArchetype Distribution:")
        for archetype, count in stats['archetype_distribution'].items():
            print(f"  {archetype}: {count}")
        print("\nFaction Distribution:")
        for faction, count in stats['faction_distribution'].items():
            print(f"  {faction}: {count}")
    
    if not any(vars(args).values()):
        parser.print_help()


if __name__ == "__main__":
    main()