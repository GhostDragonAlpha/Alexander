#!/usr/bin/env python3
"""
NPC System Core - Base classes and runtime management for Alexander VR Space Simulation

This module provides:
- NPC base class with core functionality
- Dialogue system integration
- Shop/inventory management
- Mission offer system
- Relationship/reputation tracking
- AI behavior management
"""

import json
import time
import random
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple, Any, Callable
from enum import Enum
from dataclasses import dataclass, field

class NPCState(Enum):
    """NPC activity states"""
    IDLE = "idle"
    TALKING = "talking"
    TRADING = "trading"
    MISSION_OFFER = "mission_offer"
    MOVING = "moving"
    SLEEPING = "sleeping"
    COMBAT = "combat"

class RelationshipLevel(Enum):
    """Relationship standing levels"""
    HATED = -100
    HOSTILE = -50
    UNFRIENDLY = -25
    NEUTRAL = 0
    FRIENDLY = 25
    RESPECTED = 50
    HONORED = 75
    REVERED = 100

@dataclass
class DialogueNode:
    """Represents a dialogue node/response"""
    id: str
    text: str
    conditions: Dict[str, Any] = field(default_factory=dict)
    effects: Dict[str, Any] = field(default_factory=dict)
    next_node: Optional[str] = None
    ends_conversation: bool = False
    
    def can_show(self, npc: 'NPC', player: Any) -> bool:
        """Check if this dialogue node can be shown"""
        if not self.conditions:
            return True
        
        # Check relationship requirement
        if "min_standing" in self.conditions:
            if npc.get_player_standing(player) < self.conditions["min_standing"]:
                return False
        
        # Check faction requirement
        if "required_faction" in self.conditions:
            if player.faction not in self.conditions["required_faction"]:
                return False
        
        # Check mission completion
        if "mission_completed" in self.conditions:
            mission_id = self.conditions["mission_completed"]
            if mission_id not in player.completed_missions:
                return False
        
        return True

@dataclass
class ShopItem:
    """Represents an item in NPC shop"""
    id: str
    name: str
    category: str
    price: float
    quantity: int = 1
    rarity: str = "common"
    description: str = ""
    
    def get_price(self, price_modifier: float = 1.0) -> float:
        """Get item price with modifier applied"""
        return self.price * price_modifier

@dataclass
class MissionOffer:
    """Represents a mission offered by NPC"""
    id: str
    title: str
    description: str
    mission_type: str
    difficulty: str
    duration: str
    rewards: Dict[str, Any]
    offered_by: str
    expiry_time: datetime
    requirements: Dict[str, Any] = field(default_factory=dict)
    
    def is_available(self, player: Any) -> bool:
        """Check if player meets mission requirements"""
        # Check standing requirement
        if "min_standing" in self.requirements:
            # This would need player reference to check standing
            pass
        
        # Check rank requirement
        if "rank_requirement" in self.requirements:
            if player.rank < self.requirements["rank_requirement"]:
                return False
        
        # Check skill requirement
        if "skill_requirement" in self.requirements:
            skill_type = self.requirements["skill_requirement"]["type"]
            skill_level = self.requirements["skill_requirement"]["level"]
            if getattr(player, f"{skill_type}_skill", 0) < skill_level:
                return False
        
        return True
    
    def is_expired(self) -> bool:
        """Check if mission offer has expired"""
        return datetime.now() > self.expiry_time

class NPC:
    """Base NPC class with core functionality"""
    
    def __init__(self, npc_data: Dict[str, Any]):
        self.npc_id: str = npc_data.get("NPCID", "")
        self.name: str = npc_data.get("Name", "Unknown")
        self.archetype: str = npc_data.get("Archetype", "Unknown")
        self.faction: str = npc_data.get("Faction", "Independent")
        
        # Location data
        self.location: Dict[str, str] = npc_data.get("Location", {})
        self.current_system: str = self.location.get("System", "Unknown")
        self.current_station: str = self.location.get("Station", "Unknown")
        
        # Appearance
        self.appearance: Dict[str, Any] = npc_data.get("Appearance", {})
        
        # Personality
        self.personality: Dict[str, Any] = npc_data.get("Personality", {})
        self.trait: str = self.personality.get("Trait", "Neutral")
        self.disposition: str = self.personality.get("Disposition", "Neutral")
        
        # Shop data
        self.shop_data: Dict[str, Any] = npc_data.get("Shop", {})
        self.sells_categories: List[str] = self.shop_data.get("Sells", [])
        self.buys_categories: List[str] = self.shop_data.get("Buys", [])
        self.price_modifier: float = self.shop_data.get("PriceModifier", 1.0)
        
        # Mission data
        self.mission_data: Dict[str, Any] = npc_data.get("Missions", {})
        self.mission_weight: float = self.mission_data.get("Weight", 0.3)
        self.mission_cooldown: int = self.mission_data.get("Cooldown", 300)
        self.last_mission_offer: float = 0
        
        # Relationships
        self.relationships: Dict[str, Any] = npc_data.get("Relationships", {})
        self.initial_standing: float = self.relationships.get("InitialStanding", 0)
        self.player_standings: Dict[str, float] = {}  # player_id -> standing
        
        # AI Behavior
        self.ai_behavior: Dict[str, Any] = npc_data.get("AIBehavior", {})
        self.daily_routine: List[str] = self.ai_behavior.get("DailyRoutine", [])
        self.interaction_radius: float = self.ai_behavior.get("InteractionRadius", 200)
        self.wander_radius: float = self.ai_behavior.get("WanderRadius", 50)
        self.conversation_duration: int = self.ai_behavior.get("ConversationDuration", 30)
        
        # Runtime state
        self.state: NPCState = NPCState.IDLE
        self.current_conversation: Optional[str] = None
        self.conversation_start_time: float = 0
        self.position: Tuple[float, float, float] = (0, 0, 0)
        self.last_update: float = time.time()
        
        # Dialogue system
        self.dialogue_nodes: Dict[str, DialogueNode] = {}
        self.current_dialogue_node: Optional[str] = None
        
        # Shop inventory
        self.shop_inventory: List[ShopItem] = []
        self.initialize_shop()
        
        # Mission offers
        self.active_missions: List[MissionOffer] = []
        self.initialize_missions()
        
        print(f"Initialized NPC: {self.name} ({self.archetype})")
    
    def get_player_standing(self, player: Any) -> float:
        """Get player's standing with this NPC"""
        player_id = getattr(player, "player_id", str(player))
        return self.player_standings.get(player_id, self.initial_standing)
    
    def modify_standing(self, player: Any, amount: float) -> float:
        """Modify player's standing with this NPC"""
        player_id = getattr(player, "player_id", str(player))
        current = self.get_player_standing(player)
        new_standing = max(-100, min(100, current + amount))
        self.player_standings[player_id] = new_standing
        
        # Check for relationship level changes
        old_level = self.get_relationship_level(current)
        new_level = self.get_relationship_level(new_standing)
        
        if old_level != new_level:
            self.on_relationship_change(player, old_level, new_level)
        
        return new_standing
    
    def get_relationship_level(self, standing: float) -> str:
        """Get relationship level name from standing value"""
        if standing >= 100:
            return "Revered"
        elif standing >= 75:
            return "Honored"
        elif standing >= 50:
            return "Respected"
        elif standing >= 25:
            return "Friendly"
        elif standing >= 0:
            return "Neutral"
        elif standing >= -25:
            return "Unfriendly"
        elif standing >= -50:
            return "Hostile"
        else:
            return "Hated"
    
    def on_relationship_change(self, player: Any, old_level: str, new_level: str):
        """Handle relationship level change"""
        print(f"{self.name}: Relationship with player changed from {old_level} to {new_level}")
        
        # Trigger any special events or dialogue based on relationship change
        if new_level == "Friendly":
            self.unlock_friendly_dialogue(player)
        elif new_level == "Respected":
            self.unlock_respected_dialogue(player)
        elif new_level == "Hostile":
            self.on_become_hostile(player)
    
    def unlock_friendly_dialogue(self, player: Any):
        """Unlock friendly dialogue options"""
        print(f"{self.name}: Unlocking friendly dialogue options for player")
    
    def unlock_respected_dialogue(self, player: Any):
        """Unlock respected dialogue options"""
        print(f"{self.name}: Unlocking respected dialogue options for player")
    
    def on_become_hostile(self, player: Any):
        """Handle becoming hostile to player"""
        print(f"{self.name}: Now hostile to player")
    
    def initialize_shop(self):
        """Initialize shop inventory based on archetype"""
        # This would load actual item data from game database
        common_items = {
            "Weapons": [
                ShopItem("laser_cannon", "Laser Cannon", "Weapons", 1000, 5, "common", "Basic energy weapon"),
                ShopItem("missile_launcher", "Missile Launcher", "Weapons", 2500, 3, "uncommon", "Explosive projectile weapon")
            ],
            "Shields": [
                ShopItem("basic_shield", "Basic Shield", "Shields", 800, 10, "common", "Standard energy shield"),
                ShopItem("reinforced_shield", "Reinforced Shield", "Shields", 2000, 5, "uncommon", "Enhanced protection shield")
            ],
            "Fuel": [
                ShopItem("standard_fuel", "Standard Fuel", "Fuel", 100, 50, "common", "Basic starship fuel"),
                ShopItem("premium_fuel", "Premium Fuel", "Fuel", 150, 25, "uncommon", "High-efficiency fuel")
            ]
        }
        
        # Add items based on what NPC sells
        for category in self.sells_categories:
            if category in common_items:
                self.shop_inventory.extend(common_items[category])
        
        print(f"{self.name}: Initialized shop with {len(self.shop_inventory)} items")
    
    def initialize_missions(self):
        """Initialize available missions"""
        # This would load actual mission data from game database
        mission_templates = {
            "Patrol_Mission": {
                "title": "System Patrol",
                "description": "Patrol the system and eliminate any hostile targets",
                "difficulty": "Medium",
                "duration": "Short",
                "rewards": {"credits": 5000, "reputation": 10}
            },
            "Supply_Run": {
                "title": "Supply Delivery",
                "description": "Deliver essential supplies to a remote outpost",
                "difficulty": "Easy",
                "duration": "Medium",
                "rewards": {"credits": 3000, "reputation": 5}
            },
            "Bounty_Hunt": {
                "title": "Bounty Hunt",
                "description": "Track down and eliminate a wanted criminal",
                "difficulty": "Hard",
                "duration": "Medium",
                "rewards": {"credits": 10000, "reputation": 20}
            }
        }
        
        # Generate missions based on what NPC offers
        for mission_type in self.mission_data.get("Offers", []):
            if mission_type in mission_templates:
                template = mission_templates[mission_type]
                mission = MissionOffer(
                    id=f"{self.npc_id}_{mission_type}_{int(time.time())}",
                    title=template["title"],
                    description=template["description"],
                    mission_type=mission_type,
                    difficulty=template["difficulty"],
                    duration=template["duration"],
                    rewards=template["rewards"],
                    offered_by=self.npc_id,
                    expiry_time=datetime.now() + timedelta(hours=2)
                )
                self.active_missions.append(mission)
        
        print(f"{self.name}: Initialized {len(self.active_missions)} missions")
    
    def update(self, delta_time: float):
        """Update NPC state"""
        self.last_update = time.time()
        
        # Update conversation timeout
        if self.state == NPCState.TALKING and self.current_conversation:
            conversation_duration = time.time() - self.conversation_start_time
            if conversation_duration > self.conversation_duration:
                self.end_conversation()
        
        # Update mission offers (remove expired ones)
        self.active_missions = [m for m in self.active_missions if not m.is_expired()]
        
        # AI behavior updates
        self.update_ai_behavior(delta_time)
    
    def update_ai_behavior(self, delta_time: float):
        """Update AI behavior based on current state"""
        if self.state == NPCState.IDLE:
            # Random chance to start wandering
            if random.random() < 0.1:  # 10% chance per update
                self.state = NPCState.MOVING
        elif self.state == NPCState.MOVING:
            # Simulate movement
            if random.random() < 0.3:  # 30% chance to stop moving
                self.state = NPCState.IDLE
    
    def start_conversation(self, player: Any) -> bool:
        """Start conversation with player"""
        if self.state != NPCState.IDLE:
            return False
        
        # Check if player is in interaction range
        player_pos = getattr(player, "position", (0, 0, 0))
        distance = self.get_distance_to(player_pos)
        
        if distance > self.interaction_radius:
            return False
        
        self.state = NPCState.TALKING
        self.current_conversation = player.player_id if hasattr(player, 'player_id') else str(player)
        self.conversation_start_time = time.time()
        
        print(f"{self.name}: Started conversation with player")
        return True
    
    def end_conversation(self):
        """End current conversation"""
        self.state = NPCState.IDLE
        self.current_conversation = None
        self.current_dialogue_node = None
        print(f"{self.name}: Ended conversation")
    
    def get_greeting(self, player: Any) -> str:
        """Get greeting dialogue based on relationship"""
        standing = self.get_player_standing(player)
        relationship_level = self.get_relationship_level(standing)
        
        # Get base greeting from personality
        base_greeting = self.personality.get("Greeting", "Greetings.")
        
        # Modify based on relationship
        if relationship_level == "Hostile":
            return f"What do you want, {relationship_level.lower()}?"
        elif relationship_level == "Friendly":
            return f"Good to see you again! {base_greeting}"
        elif relationship_level == "Respected":
            return f"Always a pleasure, respected ally. {base_greeting}"
        
        return base_greeting
    
    def get_dialogue_options(self, player: Any) -> List[DialogueNode]:
        """Get available dialogue options for player"""
        options = []
        
        # Add greeting option
        greeting_node = DialogueNode(
            id="greeting",
            text=self.get_greeting(player),
            next_node="main_menu"
        )
        options.append(greeting_node)
        
        # Add shop option if NPC has shop
        if self.shop_inventory:
            shop_node = DialogueNode(
                id="shop",
                text="I'd like to see what you have for sale.",
                next_node="shop_menu"
            )
            options.append(shop_node)
        
        # Add mission options if available
        if self.active_missions:
            mission_node = DialogueNode(
                id="missions",
                text="What missions do you have available?",
                next_node="mission_menu"
            )
            options.append(mission_node)
        
        # Add relationship-specific options
        standing = self.get_player_standing(player)
        if standing >= 50:  # Respected or higher
            respected_node = DialogueNode(
                id="respected_options",
                text="I need something special...",
                conditions={"min_standing": 50},
                next_node="special_menu"
            )
            options.append(respected_node)
        
        return options
    
    def get_shop_items(self, category: Optional[str] = None) -> List[ShopItem]:
        """Get shop items, optionally filtered by category"""
        if category:
            return [item for item in self.shop_inventory if item.category == category]
        return self.shop_inventory
    
    def buy_item(self, player: Any, item_id: str, quantity: int = 1) -> Tuple[bool, str]:
        """Handle player buying item from shop"""
        item = next((i for i in self.shop_inventory if i.id == item_id), None)
        if not item:
            return False, "Item not found"
        
        if item.quantity < quantity:
            return False, "Not enough quantity available"
        
        total_price = item.get_price(self.price_modifier) * quantity
        
        # Check if player has enough credits
        player_credits = getattr(player, "credits", 0)
        if player_credits < total_price:
            return False, "Not enough credits"
        
        # Process transaction
        item.quantity -= quantity
        player.credits -= total_price
        
        # Add item to player inventory
        if hasattr(player, "add_item"):
            player.add_item(item_id, quantity)
        
        # Improve relationship slightly
        self.modify_standing(player, 1)
        
        return True, f"Purchased {item.name} x{quantity} for {total_price} credits"
    
    def sell_item(self, player: Any, item_id: str, quantity: int = 1) -> Tuple[bool, str]:
        """Handle player selling item to shop"""
        # Check if player has the item
        if not hasattr(player, "has_item") or not player.has_item(item_id, quantity):
            return False, "You don't have that item"
        
        # Get item data (this would come from game database)
        item_value = 100  # Placeholder value
        sell_price = int(item_value * 0.6 * self.price_modifier)  # 60% of value
        
        total_price = sell_price * quantity
        
        # Process transaction
        player.add_item(item_id, -quantity)  # Remove from player
        player.credits += total_price
        
        # Add to shop inventory
        existing_item = next((i for i in self.shop_inventory if i.id == item_id), None)
        if existing_item:
            existing_item.quantity += quantity
        else:
            # Create new shop item
            new_item = ShopItem(
                id=item_id,
                name=item_id.replace("_", " ").title(),
                category="Bought",
                price=sell_price * 1.5,  # Sell for 150% of buy price
                quantity=quantity
            )
            self.shop_inventory.append(new_item)
        
        return True, f"Sold item for {total_price} credits"
    
    def offer_mission(self, player: Any, mission_id: str) -> Tuple[bool, str]:
        """Offer mission to player"""
        mission = next((m for m in self.active_missions if m.id == mission_id), None)
        if not mission:
            return False, "Mission not found"
        
        if not mission.is_available(player):
            return False, "You don't meet the requirements for this mission"
        
        if mission.is_expired():
            return False, "This mission offer has expired"
        
        # Add mission to player's mission log
        if hasattr(player, "add_mission"):
            player.add_mission(mission)
        
        # Remove from active missions
        self.active_missions.remove(mission)
        
        # Set cooldown for new missions
        self.last_mission_offer = time.time()
        
        return True, f"Mission '{mission.title}' added to your log"
    
    def get_distance_to(self, position: Tuple[float, float, float]) -> float:
        """Calculate distance to position"""
        # Simple 3D distance calculation
        dx = self.position[0] - position[0]
        dy = self.position[1] - position[1]
        dz = self.position[2] - position[2]
        return (dx**2 + dy**2 + dz**2)**0.5
    
    def can_interact(self, player: Any) -> bool:
        """Check if player can interact with this NPC"""
        if self.state in [NPCState.TALKING, NPCState.COMBAT]:
            return False
        
        # Check distance
        player_pos = getattr(player, "position", None)
        if player_pos:
            distance = self.get_distance_to(player_pos)
            if distance > self.interaction_radius:
                return False
        
        # Check if NPC is on cooldown
        if self.state == NPCState.SLEEPING:
            return False
        
        return True
    
    def get_debug_info(self) -> Dict[str, Any]:
        """Get debug information about NPC"""
        return {
            "npc_id": self.npc_id,
            "name": self.name,
            "archetype": self.archetype,
            "state": self.state.value,
            "location": self.location,
            "shop_items": len(self.shop_inventory),
            "active_missions": len(self.active_missions),
            "personality": self.personality,
            "ai_behavior": self.ai_behavior
        }


class NPCManager:
    """Manages all NPCs in the game world"""
    
    def __init__(self, config_path: str = "Config/NPCConfig.json"):
        self.config_path = config_path
        self.npcs: Dict[str, NPC] = {}
        self.config = self._load_config()
        self.last_update: float = time.time()
        
    def _load_config(self) -> Dict[str, Any]:
        """Load NPC configuration"""
        try:
            with open(self.config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            print(f"Error loading NPC config: {e}")
            return {}
    
    def spawn_npc(self, npc_data: Dict[str, Any]) -> Optional[NPC]:
        """Spawn a new NPC from data"""
        try:
            npc = NPC(npc_data)
            self.npcs[npc.npc_id] = npc
            print(f"Spawned NPC: {npc.name} ({npc.archetype})")
            return npc
        except Exception as e:
            print(f"Error spawning NPC: {e}")
            return None
    
    def spawn_npc_from_file(self, filepath: str) -> Optional[NPC]:
        """Spawn NPC from JSON file"""
        try:
            with open(filepath, 'r') as f:
                npc_data = json.load(f)
            return self.spawn_npc(npc_data)
        except Exception as e:
            print(f"Error loading NPC from file {filepath}: {e}")
            return None
    
    def despawn_npc(self, npc_id: str) -> bool:
        """Remove NPC from world"""
        if npc_id in self.npcs:
            npc = self.npcs[npc_id]
            print(f"Despawned NPC: {npc.name}")
            del self.npcs[npc_id]
            return True
        return False
    
    def get_npc(self, npc_id: str) -> Optional[NPC]:
        """Get NPC by ID"""
        return self.npcs.get(npc_id)
    
    def get_npcs_by_archetype(self, archetype: str) -> List[NPC]:
        """Get all NPCs of specific archetype"""
        return [npc for npc in self.npcs.values() if npc.archetype == archetype]
    
    def get_npcs_by_location(self, system: str, station: Optional[str] = None) -> List[NPC]:
        """Get NPCs at specific location"""
        if station:
            return [npc for npc in self.npcs.values() 
                   if npc.current_system == system and npc.current_station == station]
        else:
            return [npc for npc in self.npcs.values() if npc.current_system == system]
    
    def update(self, delta_time: float):
        """Update all NPCs"""
        for npc in self.npcs.values():
            npc.update(delta_time)
    
    def get_nearby_npcs(self, position: Tuple[float, float, float], radius: float) -> List[NPC]:
        """Get NPCs within radius of position"""
        nearby = []
        for npc in self.npcs.values():
            distance = npc.get_distance_to(position)
            if distance <= radius:
                nearby.append(npc)
        return nearby
    
    def get_interactable_npcs(self, player: Any) -> List[NPC]:
        """Get NPCs that player can interact with"""
        player_pos = getattr(player, "position", (0, 0, 0))
        nearby_npcs = self.get_nearby_npcs(player_pos, 500)  # Large search radius
        
        interactable = [npc for npc in nearby_npcs if npc.can_interact(player)]
        return interactable
    
    def load_npcs_from_directory(self, directory: str = "Content/NPCs") -> int:
        """Load all NPC files from directory"""
        import os
        count = 0
        
        for filename in os.listdir(directory):
            if filename.startswith("NPC_") and filename.endswith(".json"):
                filepath = os.path.join(directory, filename)
                if self.spawn_npc_from_file(filepath):
                    count += 1
        
        print(f"Loaded {count} NPCs from {directory}")
        return count
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get NPC system statistics"""
        archetype_counts = {}
        faction_counts = {}
        state_counts = {}
        
        for npc in self.npcs.values():
            archetype_counts[npc.archetype] = archetype_counts.get(npc.archetype, 0) + 1
            faction_counts[npc.faction] = faction_counts.get(npc.faction, 0) + 1
            state_counts[npc.state.value] = state_counts.get(npc.state.value, 0) + 1
        
        return {
            "total_npcs": len(self.npcs),
            "archetype_distribution": archetype_counts,
            "faction_distribution": faction_counts,
            "state_distribution": state_counts
        }


# Global NPC manager instance
npc_manager = NPCManager()

def initialize_npc_system():
    """Initialize the NPC system"""
    print("Initializing NPC System...")
    
    # Load NPCs from files
    count = npc_manager.load_npcs_from_directory()
    
    # Print statistics
    stats = npc_manager.get_statistics()
    print(f"NPC System initialized with {stats['total_npcs']} NPCs")
    print("Archetype distribution:")
    for archetype, count in stats['archetype_distribution'].items():
        print(f"  {archetype}: {count}")
    
    return npc_manager

def update_npc_system(delta_time: float):
    """Update NPC system (call each frame)"""
    npc_manager.update(delta_time)

def get_npc_system() -> NPCManager:
    """Get the global NPC manager"""
    return npc_manager

if __name__ == "__main__":
    # Test the NPC system
    print("Testing NPC System...")
    
    # Initialize
    manager = initialize_npc_system()
    
    # Print detailed stats
    stats = manager.get_statistics()
    print(f"\nDetailed Statistics:")
    print(json.dumps(stats, indent=2))
    
    # Test getting interactable NPCs (with mock player)
    class MockPlayer:
        def __init__(self):
            self.player_id = "test_player"
            self.position = (0, 0, 0)
            self.credits = 10000
    
    player = MockPlayer()
    interactable = manager.get_interactable_npcs(player)
    print(f"\nInteractable NPCs: {len(interactable)}")
    for npc in interactable[:3]:  # Show first 3
        print(f"  - {npc.name} ({npc.archetype})")