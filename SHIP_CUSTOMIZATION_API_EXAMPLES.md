# Ship Customization API Testing Examples

This document provides example curl commands for testing the new ship customization endpoints added to AutomationAPIServer.

## Prerequisites

1. Start the Unreal Engine server with AutomationAPIServer component enabled
2. Server should be listening on port 8080 (default)
3. Spawn a ship first to get a ship_id

## Endpoint Examples

### 1. Apply Ship Customization (POST /apply_ship_customization)

Apply customization stats to a ship's ShipCustomizationComponent.

**Request:**
```bash
curl -X POST http://localhost:8080/apply_ship_customization \
  -H "Content-Type: application/json" \
  -d '{
    "ship_id": "ship_1",
    "mass": 1500.0,
    "thrust_power": 2.5,
    "max_velocity": 1.8,
    "rotation_speed": 1.2,
    "acceleration": 1.5
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Ship customization applied",
  "timestamp": "2025-11-07T20:54:05.000Z",
  "data": {
    "ship_id": "ship_1",
    "mass": 1500.0,
    "thrust_power": 2.5,
    "max_velocity": 1.8,
    "rotation_speed": 1.2,
    "acceleration": 1.5
  }
}
```

### 2. Get Ship Customization (GET /get_ship_customization/{ship_id})

Get current customization stats from a ship.

**Request:**
```bash
curl http://localhost:8080/get_ship_customization/ship_1
```

**Response:**
```json
{
  "success": true,
  "message": "Ship customization retrieved",
  "timestamp": "2025-11-07T20:54:05.000Z",
  "data": {
    "ship_id": "ship_1",
    "mass": 1500.0,
    "thrust_power": 2.5,
    "max_velocity": 1.8,
    "rotation_speed": 1.2,
    "acceleration": 1.5,
    "hull_integrity": 100.0,
    "shield_strength": 0.0,
    "weapon_damage": 1.0,
    "energy_capacity": 100.0,
    "energy_regen_rate": 10.0
  }
}
```

### 3. Equip Ship Part (POST /equip_ship_part)

Equip a part in a specific slot on the ship.

**Part Categories (int values):**
- 0 = Engine
- 1 = Thrusters
- 2 = Hull
- 3 = Wings
- 4 = Cockpit
- 5 = Weapon
- 6 = Shield
- 7 = Utility

**Request:**
```bash
curl -X POST http://localhost:8080/equip_ship_part \
  -H "Content-Type: application/json" \
  -d '{
    "ship_id": "ship_1",
    "part_category": 0,
    "part_id": "EngineBasic"
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Ship part equipped",
  "timestamp": "2025-11-07T20:54:05.000Z",
  "data": {
    "ship_id": "ship_1",
    "equipped_part_id": "EngineBasic",
    "part_category": 0,
    "equipped_parts": {
      "Engine": "EngineBasic",
      "Thrusters": "ThrustersDefault",
      "Hull": "HullStandard"
    },
    "total_stats": {
      "mass": 1500.0,
      "thrust_power": 2.5,
      "max_velocity": 1.8,
      "rotation_speed": 1.2
    }
  }
}
```

### 4. Get Ship Loadout (GET /get_ship_loadout/{ship_id})

Get complete loadout configuration for a ship.

**Request:**
```bash
curl http://localhost:8080/get_ship_loadout/ship_1
```

**Response:**
```json
{
  "success": true,
  "message": "Ship loadout retrieved",
  "timestamp": "2025-11-07T20:54:05.000Z",
  "data": {
    "ship_id": "ship_1",
    "loadout_name": "Default Loadout",
    "equipped_skin": "DefaultSkin",
    "equipped_parts": {
      "Engine": "EngineBasic",
      "Thrusters": "ThrustersDefault",
      "Hull": "HullStandard",
      "Wings": "WingsStandard"
    },
    "total_stats": {
      "mass": 1500.0,
      "thrust_power": 2.5,
      "max_velocity": 1.8,
      "rotation_speed": 1.2,
      "acceleration": 1.5,
      "hull_integrity": 100.0,
      "shield_strength": 0.0,
      "weapon_damage": 1.0,
      "energy_capacity": 100.0,
      "energy_regen_rate": 10.0
    }
  }
}
```

## Complete Testing Workflow

Here's a complete workflow to test ship customization:

```bash
# 1. Check server status
curl http://localhost:8080/status

# 2. Spawn a ship
curl -X POST http://localhost:8080/spawn_ship \
  -H "Content-Type: application/json" \
  -d '{
    "location": [0, 0, 500],
    "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
  }'

# 3. Get initial customization stats
curl http://localhost:8080/get_ship_customization/ship_1

# 4. Apply custom stats
curl -X POST http://localhost:8080/apply_ship_customization \
  -H "Content-Type: application/json" \
  -d '{
    "ship_id": "ship_1",
    "mass": 2000.0,
    "thrust_power": 3.0,
    "max_velocity": 2.0
  }'

# 5. Verify stats were applied
curl http://localhost:8080/get_ship_customization/ship_1

# 6. Equip a part (if data table exists)
curl -X POST http://localhost:8080/equip_ship_part \
  -H "Content-Type: application/json" \
  -d '{
    "ship_id": "ship_1",
    "part_category": 0,
    "part_id": "EngineAdvanced"
  }'

# 7. Get complete loadout
curl http://localhost:8080/get_ship_loadout/ship_1

# 8. List all ships
curl http://localhost:8080/list_ships
```

## Python Testing Examples

```python
import requests

BASE_URL = "http://localhost:8080"

# Spawn a ship
response = requests.post(f"{BASE_URL}/spawn_ship", json={
    "location": [0, 0, 500],
    "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
})
ship_data = response.json()
ship_id = ship_data["data"]["ship_id"]
print(f"Spawned ship: {ship_id}")

# Apply customization
requests.post(f"{BASE_URL}/apply_ship_customization", json={
    "ship_id": ship_id,
    "mass": 1800.0,
    "thrust_power": 2.8,
    "max_velocity": 2.2
})
print("Applied customization")

# Get stats
response = requests.get(f"{BASE_URL}/get_ship_customization/{ship_id}")
stats = response.json()
print(f"Current stats: {stats['data']}")

# Equip part
requests.post(f"{BASE_URL}/equip_ship_part", json={
    "ship_id": ship_id,
    "part_category": 0,  # Engine
    "part_id": "EngineBasic"
})
print("Equipped engine part")

# Get loadout
response = requests.get(f"{BASE_URL}/get_ship_loadout/{ship_id}")
loadout = response.json()
print(f"Loadout: {loadout['data']}")
```

## Error Handling

All endpoints return standard error responses:

```json
{
  "success": false,
  "message": "Error description",
  "timestamp": "2025-11-07T20:54:05.000Z"
}
```

Common errors:
- `"Ship not found: ship_X"` - Invalid ship_id
- `"Ship has no ShipCustomizationComponent"` - Ship doesn't have the component
- `"Invalid JSON"` - Malformed request body
- `"No customization stats provided"` - Missing required parameters
- `"Failed to equip part: PartName"` - Part doesn't exist in data table

## Notes

1. **Ship Requirements**: The ship must have a `ShipCustomizationComponent` attached for these endpoints to work.

2. **Part IDs**: Part IDs must match entries in the ShipPartsDataTable configured in the component.

3. **Category Values**: Part categories are integers (0-7) corresponding to the EShipPartCategory enum.

4. **Stats Application**: The `apply_ship_customization` endpoint directly modifies stats and applies them to the FlightController. This bypasses the part system for testing purposes.

5. **Loadout Persistence**: Changes are not automatically saved. The ShipCustomizationComponent has save/load methods that can be called separately.

## Integration with Automation

These endpoints are designed for automated testing workflows:

```python
# Example: Test different ship configurations
test_configs = [
    {"mass": 1000, "thrust_power": 2.0, "max_velocity": 1.5},
    {"mass": 2000, "thrust_power": 3.0, "max_velocity": 2.0},
    {"mass": 3000, "thrust_power": 4.0, "max_velocity": 2.5},
]

for config in test_configs:
    # Apply configuration
    requests.post(f"{BASE_URL}/apply_ship_customization", json={
        "ship_id": "ship_1",
        **config
    })

    # Test flight performance
    # ... perform flight tests ...

    # Record results
    # ... save metrics ...
```
