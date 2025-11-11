# NPC Interaction UI Specification
## Alexander VR Space Simulation

### Overview

This document specifies the UI system for NPC interactions in the Alexander VR Space Simulation. The UI is designed for VR environments with considerations for readability, immersion, and ease of use in virtual reality.

---

## 1. UI Architecture

### 1.1 Core UI Components

```json
{
  "NPC_UI_System": {
    "Components": {
      "Dialogue_Widget": "Main dialogue interaction interface",
      "Shop_Widget": "Trading and inventory management",
      "Mission_Widget": "Mission offers and management",
      "Relationship_Widget": "Reputation and standing display",
      "NPC_Info_Widget": "NPC information and status"
    },
    "VR_Considerations": {
      "Text_Size": "Large enough for VR readability (32-48pt)",
      "Contrast": "High contrast for visibility",
      "Distance": "1.5-2m from player in VR space",
      "Angle": "Slight downward angle (15-20 degrees)",
      "Interaction": "Gaze-based or controller pointer"
    }
  }
}
```

### 1.2 UI Flow

```
Player Approaches NPC → Proximity Detection → UI Prompt Appears
    ↓
Player Activates Interaction → Main Dialogue UI Opens
    ↓
Player Chooses Option:
    ├── Dialogue → Dialogue Tree
    ├── Trade → Shop UI
    ├── Missions → Mission UI
    └── Information → NPC Info UI
```

---

## 2. Dialogue UI System

### 2.1 Main Dialogue Widget

**Layout Structure:**
```
┌─────────────────────────────────────────┐
│  [NPC Name] - [Archetype]              │
│  [Relationship Level] [Faction]        │
├─────────────────────────────────────────┤
│                                         │
│  NPC Portrait (Left Side)              │
│  ┌──────────────────────────────────┐  │
│  │  NPC Dialogue Text               │  │
│  │  (Animated typewriter effect)    │  │
│  └──────────────────────────────────┘  │
│                                         │
│  Player Response Options:              │
│  ┌──────────────────────────────────┐  │
│  │  > [Response Option 1]           │  │
│  │  > [Response Option 2]           │  │
│  │  > [Response Option 3]           │  │
│  │  > [End Conversation]            │  │
│  └──────────────────────────────────┘  │
│                                         │
│  [Relationship: Friendly] [Credits]    │
└─────────────────────────────────────────┘
```

**VR Implementation Details:**
- **Size**: 800x600 pixels in VR space (scaled appropriately)
- **Position**: 1.5m in front of player, 1.7m height
- **Background**: Semi-transparent dark panel (alpha 0.8)
- **Text**: White text with drop shadow for readability
- **Animation**: Smooth fade-in/out transitions

### 2.2 Dialogue Node Structure

```json
{
  "Dialogue_Node": {
    "Node_ID": "ship_dealer_greeting_01",
    "NPC_Text": "Welcome to Calhoun's Ships! Best deals in the sector!",
    "Player_Options": [
      {
        "Option_ID": "browse_ships",
        "Text": "Show me your ships",
        "Next_Node": "ship_dealer_shop_01",
        "Requirements": {
          "Min_Standing": -25
        },
        "Effects": {
          "Relationship_Change": 1
        }
      },
      {
        "Option_ID": "ask_missions",
        "Text": "Do you have any work for me?",
        "Next_Node": "ship_dealer_missions_01",
        "Requirements": {
          "Min_Standing": 0
        }
      },
      {
        "Option_ID": "end_conversation",
        "Text": "Maybe later",
        "Next_Node": "farewell",
        "Ends_Conversation": true
      }
    ],
    "Animations": {
      "NPC_Gesture": "WELCOME_GESTURE",
      "Facial_Expression": "FRIENDLY_SMILE"
    }
  }
}
```

### 2.3 Dynamic Dialogue System

**Relationship-Based Responses:**
- **Hated/Hostile**: "Get away from me, scum!"
- **Unfriendly**: "What do you want? Make it quick."
- **Neutral**: "Greetings. How can I help you?"
- **Friendly**: "Good to see you again! What can I do for you?"
- **Respected**: "Always a pleasure, my friend. What do you need?"
- **Honored**: "Welcome, honored ally! How may I assist you?"
- **Revered**: "My lord/lady! It is an honor to serve you."

---

## 3. Shop/Trading UI

### 3.1 Shop Widget Layout

```
┌─────────────────────────────────────────────────────────────┐
│  [NPC Name]'s Shop - [Shop Type]                          │
│  [Your Credits: 15,250 CR]                                  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────────┐  ┌────────────────────────────────┐ │
│  │  CATEGORIES      │  │  ITEM LIST                     │ │
│  │                  │  │                                │ │
│  │  > Weapons       │  │  Laser Cannon          1,000 │ │
│  │  > Shields       │  │  Missile Launcher      2,500 │ │
│  │  > Engines       │  │  Plasma Gun            1,800 │ │
│  │  > Components    │  │                                │ │
│  │  > [SELL]        │  │  [Item Description]            │ │
│  │                  │  │  High energy weapon...         │ │
│  └──────────────────┘  └────────────────────────────────┘ │
│                                                             │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  SELECTED ITEM: Laser Cannon                          │ │
│  │  Price: 1,000 CR (Your offer: 800 CR)                 │ │
│  │  [BUY] [HAGGLE] [CANCEL]                              │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                             │
│  [Relationship Bonus: -5%] [Faction Discount: -10%]        │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Trading Mechanics

**Price Calculation:**
```python
final_price = base_price * npc_price_modifier * relationship_modifier * faction_modifier

# Example:
base_price = 1000
npc_price_modifier = 1.0  # From NPC data
relationship_modifier = 0.95  # 5% discount for friendly
faction_modifier = 0.90  # 10% faction discount

final_price = 1000 * 1.0 * 0.95 * 0.90 = 855 credits
```

**Haggling System:**
- **Success Chance**: Based on Charisma/Social skill + Relationship level
- **Success**: 10-25% price reduction
- **Failure**: 5-10% price increase, slight relationship penalty
- **Critical Failure**: NPC refuses to trade for a period

---

## 4. Mission UI System

### 4.1 Mission Offer Widget

```
┌─────────────────────────────────────────────────────────────┐
│  Mission: Patrol Duty                                       │
│  Offered by: Admiral Sarah Kane                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Description:                                               │
│  Patrol the Alpha Centauri system and eliminate any        │
│  hostile targets that threaten civilian shipping.          │
│                                                             │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Objectives:                                           │ │
│  │  • Patrol 3 waypoints                                  │ │
│  │  • Eliminate hostile ships (0/3)                       │ │
│  │  • Return to station                                   │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                             │
│  Difficulty: ★★★☆☆ (Medium)                                │
│  Time Limit: 2 hours                                       │
│                                                             │
│  Rewards:                                                   │
│  • Credits: 5,000 CR                                       │
│  • Reputation: +10 (United Federation)                     │
│  • Experience: 250 XP                                      │
│  • Bonus: Rare weapon component                            │
│                                                             │
│  Requirements:                                             │
│  • Combat Rank: Recruit or higher                          │
│  • Ship Class: Fighter or larger                           │
│                                                             │
│  [ACCEPT MISSION] [DECLINE] [MORE INFO]                    │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Mission Tracking Integration

**Active Mission Display:**
```
┌─────────────────────────────────────────────────┐
│  Active Missions (3)                           │
├─────────────────────────────────────────────────┤
│  ▶ Patrol Duty (Admiral Kane)                  │
│    Progress: 2/3 waypoints                     │
│    Time: 1:23:45 remaining                     │
│                                                 │
│  ▶ Supply Run (Mira Chen)                      │
│    Progress: Deliver cargo (0/50 units)        │
│    Time: 3:15:20 remaining                     │
│                                                 │
│  ▶ [+ Track New Mission]                       │
└─────────────────────────────────────────────────┘
```

---

## 5. Relationship/Reputation UI

### 5.1 Relationship Widget

```
┌─────────────────────────────────────────────────┐
│  Your Standing with Factions                   │
├─────────────────────────────────────────────────┤
│                                                 │
│  United Federation                              │
│  [████████████░░░░░░░░] 60% - Friendly         │
│  Reputation: Respected                          │
│                                                 │
│  Trade Guild                                    │
│  [████████░░░░░░░░░░░░] 40% - Neutral          │
│  Reputation: Neutral                            │
│                                                 │
│  Science Corps                                  │
│  [████████████████████] 95% - Honored          │
│  Reputation: Honored                            │
│                                                 │
│  Syndicate (Hidden)                            │
│  [███░░░░░░░░░░░░░░░░░] -15% - Unfriendly      │
│  Reputation: Hostile                            │
│                                                 │
│  [View Detailed Report] [Faction Benefits]     │
└─────────────────────────────────────────────────┘
```

### 5.2 NPC-Specific Relationship

```
┌─────────────────────────────────────────────────┐
│  Relationship: Admiral Sarah Kane              │
├─────────────────────────────────────────────────┤
│                                                 │
│  Current Standing: 65/100 - Respected          │
│  [████████████████████░░]                      │
│                                                 │
│  Faction: United Federation                     │
│  Archetype: Mission Commander                   │
│                                                 │
│  Recent Activity:                              │
│  • Mission completed: +10 rep                  │
│  • Friendly dialogue: +2 rep                   │
│  • Help in combat: +15 rep                     │
│                                                 │
│  Benefits Unlocked:                            │
│  ✓ Access to military missions                 │
│  ✓ Discount on equipment (10%)                 │
│  ✓ Priority docking access                     │
│  ⏳ Elite missions at 75 rep                   │
│  ⏳ Special equipment at 90 rep                │
│                                                 │
│  [View Mission History] [Diplomatic Actions]   │
└─────────────────────────────────────────────────┘
```

---

## 6. NPC Information UI

### 6.1 NPC Profile Widget

```
┌─────────────────────────────────────────────────┐
│  [NPC Portrait]                                │
│  Admiral Sarah Kane                            │
│  Mission Commander                             │
├─────────────────────────────────────────────────┤
│                                                 │
│  Faction: United Federation                     │
│  Location: Earth Spacestation, Sol System      │
│                                                 │
│  Personality: Authoritative, Professional      │
│  Disposition: Professional                     │
│                                                 │
│  Background:                                   │
│  Decorated military officer with 20 years      │
│  of service. Known for tactical brilliance     │
│  and strict adherence to protocol.             │
│                                                 │
│  Services:                                     │
│  ✓ Military missions                           │
│  ✓ Strategic briefings                         │
│  ✓ Equipment procurement                       │
│                                                 │
│  [Start Conversation] [View Missions]          │
└─────────────────────────────────────────────────┘
```

---

## 7. VR-Specific UI Features

### 7.1 Gaze-Based Interaction

**Implementation:**
```python
# Pseudo-code for gaze detection
def update_gaze_interaction(player_hmd_position, player_hmd_rotation):
    # Cast ray from HMD position in look direction
    gaze_ray = calculate_ray(player_hmd_position, player_hmd_rotation)
    
    # Check for NPC intersection
    for npc in nearby_npcs:
        if ray_intersects_npc(gaze_ray, npc):
            # Show interaction prompt after 1 second of gaze
            if gaze_duration > 1.0:
                show_interaction_prompt(npc)
            
            # Activate interaction after 2 seconds or button press
            if gaze_duration > 2.0 or activation_button_pressed:
                start_npc_interaction(npc)
```

### 7.2 Controller Pointer Interaction

**Laser Pointer UI:**
- **Visual**: Blue laser pointer from controller
- **Selection**: Red dot on UI elements when hovered
- **Activation**: Trigger pull or A/B button press
- **Feedback**: Haptic feedback on selection

### 7.3 Voice Command Integration

**Supported Commands:**
- "Hello [NPC Name]" - Start conversation
- "Show me your wares" - Open shop
- "What missions do you have?" - View missions
- "Goodbye" - End conversation
- "Accept mission" - Accept current mission offer

---

## 8. UI Integration with Python Systems

### 8.1 Data Flow Architecture

```python
# Python NPC System → Unreal UI Integration

class NPC_UI_Bridge:
    def __init__(self, npc_manager):
        self.npc_manager = npc_manager
        self.current_npc = None
        self.active_widget = None
    
    def start_npc_interaction(self, npc_id, player):
        """Called when player initiates NPC interaction"""
        npc = self.npc_manager.get_npc(npc_id)
        if not npc:
            return False
        
        self.current_npc = npc
        
        # Check if interaction is possible
        if not npc.can_interact(player):
            return False
        
        # Start conversation
        npc.start_conversation(player)
        
        # Send data to Unreal UI
        self.send_dialogue_data(npc, player)
        
        return True
    
    def send_dialogue_data(self, npc, player):
        """Send dialogue data to Unreal UI"""
        dialogue_data = {
            "npc_name": npc.name,
            "npc_archetype": npc.archetype,
            "npc_portrait": npc.appearance.get("Model", "default"),
            "greeting_text": npc.get_greeting(player),
            "dialogue_options": [
                {
                    "id": option.id,
                    "text": option.text,
                    "enabled": option.can_show(npc, player)
                }
                for option in npc.get_dialogue_options(player)
            ],
            "relationship_level": npc.get_relationship_level(
                npc.get_player_standing(player)
            ),
            "player_credits": getattr(player, "credits", 0)
        }
        
        # Send to Unreal via your preferred method
        # (HTTP API, socket, file-based communication, etc.)
        self.send_to_unreal("dialogue_ui", dialogue_data)
    
    def handle_dialogue_selection(self, option_id, player):
        """Handle player dialogue selection"""
        if not self.current_npc:
            return False
        
        # Process dialogue option
        if option_id == "shop":
            self.open_shop_ui(self.current_npc, player)
        elif option_id == "missions":
            self.open_mission_ui(self.current_npc, player)
        elif option_id == "end_conversation":
            self.end_interaction()
        else:
            # Handle specific dialogue nodes
            pass
        
        return True
    
    def open_shop_ui(self, npc, player):
        """Open shop interface"""
        shop_data = {
            "npc_name": npc.name,
            "shop_categories": npc.sells_categories,
            "inventory": [
                {
                    "id": item.id,
                    "name": item.name,
                    "category": item.category,
                    "price": item.get_price(npc.price_modifier),
                    "quantity": item.quantity,
                    "description": item.description
                }
                for item in npc.get_shop_items()
            ],
            "player_credits": player.credits,
            "relationship_bonus": self.calculate_relationship_discount(npc, player)
        }
        
        self.send_to_unreal("shop_ui", shop_data)
        self.active_widget = "shop"
    
    def open_mission_ui(self, npc, player):
        """Open mission offers interface"""
        mission_data = {
            "npc_name": npc.name,
            "missions": [
                {
                    "id": mission.id,
                    "title": mission.title,
                    "description": mission.description,
                    "difficulty": mission.difficulty,
                    "duration": mission.duration,
                    "rewards": mission.rewards,
                    "requirements": mission.requirements,
                    "available": mission.is_available(player),
                    "expiry_time": mission.expiry_time.isoformat()
                }
                for mission in npc.active_missions
            ]
        }
        
        self.send_to_unreal("mission_ui", mission_data)
        self.active_widget = "missions"
    
    def handle_shop_transaction(self, transaction_type, item_id, quantity, player):
        """Handle shop buy/sell transaction"""
        if not self.current_npc:
            return False
        
        if transaction_type == "buy":
            success, message = self.current_npc.buy_item(player, item_id, quantity)
        elif transaction_type == "sell":
            success, message = self.current_npc.sell_item(player, item_id, quantity)
        else:
            return False
        
        # Update UI with result
        result_data = {
            "success": success,
            "message": message,
            "updated_credits": player.credits
        }
        
        self.send_to_unreal("transaction_result", result_data)
        
        # Refresh shop UI if successful
        if success:
            self.open_shop_ui(self.current_npc, player)
        
        return success
    
    def handle_mission_acceptance(self, mission_id, player):
        """Handle mission acceptance"""
        if not self.current_npc:
            return False
        
        success, message = self.current_npc.offer_mission(player, mission_id)
        
        result_data = {
            "success": success,
            "message": message
        }
        
        self.send_to_unreal("mission_acceptance_result", result_data)
        return success
    
    def end_interaction(self):
        """End NPC interaction"""
        if self.current_npc:
            self.current_npc.end_conversation()
            self.current_npc = None
        
        self.send_to_unreal("close_all_ui", {})
        self.active_widget = None
    
    def send_to_unreal(self, message_type, data):
        """Send data to Unreal Engine"""
        # Implementation depends on your Unreal-Python communication method
        # Options:
        # 1. HTTP API calls to Unreal web server
        # 2. Socket communication
        # 3. File-based communication
        # 4. Database/shared memory
        
        # Example using HTTP:
        # requests.post("http://localhost:8080/ui/update", json={
        #     "type": message_type,
        #     "data": data
        # })
        
        # For now, just print
        print(f"UI Bridge: {message_type} - {json.dumps(data, indent=2)}")
```

---

## 9. UI Assets and Resources

### 9.1 Required UI Assets

**Textures:**
- `UI/Dialogue/Dialogue_Panel.png` - Main dialogue background
- `UI/Shop/Shop_Panel.png` - Shop interface background
- `UI/Missions/Mission_Panel.png` - Mission UI background
- `UI/Relationship/Rep_Panel.png` - Relationship UI background
- `UI/Buttons/Button_Normal.png` - Normal button state
- `UI/Buttons/Button_Hovered.png` - Hovered button state
- `UI/Buttons/Button_Pressed.png` - Pressed button state

**Fonts:**
- `UI/Fonts/Main_Font.ttf` - Primary UI font (32-48pt for VR)
- `UI/Fonts/Header_Font.ttf` - Headers and titles (48-64pt for VR)

**Icons:**
- `UI/Icons/Credits_Icon.png` - Currency icon
- `UI/Icons/Reputation_Icon.png` - Reputation icon
- `UI/Icons/Standing_Bar.png` - Relationship standing bar
- `UI/Icons/Difficulty_Star.png` - Mission difficulty stars

### 9.2 Animation Assets

**Dialogue Animations:**
- `UI/Animations/Dialogue_Open.uasset` - UI open animation
- `UI/Animations/Dialogue_Close.uasset` - UI close animation
- `UI/Animations/Text_Typewriter.uasset` - Text reveal effect
- `UI/Animations/Button_Press.uasset` - Button press feedback

---

## 10. Performance Considerations

### 10.1 VR Performance Optimization

**UI Rendering:**
- **Draw Calls**: Combine UI elements to minimize draw calls
- **Texture Size**: Use appropriate texture sizes (512x512 for most UI elements)
- **Transparency**: Minimize overlapping transparent elements
- **Update Frequency**: Only update UI when data changes

**Memory Management:**
- **UI Pooling**: Reuse UI widgets instead of creating/destroying
- **Texture Streaming**: Stream UI textures as needed
- **Garbage Collection**: Avoid frequent UI object creation

### 10.2 Network Optimization (Multiplayer)

**Data Compression:**
- Send only changed UI data
- Use integer IDs instead of strings where possible
- Compress large UI updates

**Update Rate:**
- UI updates: 30 FPS maximum
- Player input: Immediate response
- NPC state: Update only when changed

---

## 11. Testing Checklist

### 11.1 UI Functionality Tests

- [ ] All UI elements render correctly in VR
- [ ] Text is readable at 1.5m distance
- [ ] Buttons respond to gaze and controller input
- [ ] UI scales properly with different VR headsets
- [ ] No UI clipping or overlap issues
- [ ] Smooth animations without frame drops

### 11.2 Integration Tests

- [ ] Dialogue system connects to Python backend
- [ ] Shop transactions update player inventory
- [ ] Mission offers appear correctly
- [ ] Relationship changes reflect in UI
- [ ] NPC data loads from JSON files
- [ ] UI updates reflect game state changes

### 11.3 VR Comfort Tests

- [ ] UI doesn't cause motion sickness
- [ ] UI distance is comfortable (1.5-2m)
- [ ] UI angle doesn't strain neck
- [ ] Text size is readable without leaning forward
- [ ] UI doesn't obstruct gameplay view

---

## 12. Future Enhancements

### 12.1 Planned Features

**Advanced Dialogue:**
- Voice recognition integration
- Context-aware responses
- Memory of past conversations
- Dynamic personality changes

**Enhanced Shop:**
- 3D item previews
- Item comparison system
- Bulk trading interface
- Auction house integration

**Social Features:**
- Player reputation sharing
- NPC gossip system
- Dynamic faction relationships
- Multiplayer NPC interactions

---

## 13. Implementation Notes

### 13.1 Unreal Engine Integration

**Blueprint Integration:**
- Create UMG Widgets for each UI component
- Expose Python bridge functions to Blueprint
- Use Event Dispatchers for UI updates
- Implement interface for NPC interaction

**C++ Integration (Optional):**
- Create NPC UI Manager class
- Implement UI data structures
- Optimize performance-critical UI code
- Expose to Blueprint for designer access

### 13.2 Python-Unreal Communication

**Recommended Approach:**
1. **HTTP REST API**: Unreal runs web server, Python makes requests
2. **WebSocket**: Real-time bidirectional communication
3. **File-based**: JSON files for data exchange (simple but slower)
4. **Shared Memory**: For high-performance requirements

**Example HTTP API Endpoints:**
```
POST /ui/dialogue/start
POST /ui/dialogue/select
POST /ui/shop/buy
POST /ui/shop/sell
POST /ui/mission/accept
GET  /ui/npc/info
```

---

This specification provides a complete foundation for implementing the NPC interaction UI system in Unreal Engine for the Alexander VR Space Simulation. The system is designed to be modular, extensible, and optimized for VR environments while maintaining strong integration with the Python backend systems.