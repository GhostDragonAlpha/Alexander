# Alexander: Infinite Horizons - API Documentation

## Overview

This document provides comprehensive technical API documentation for all major systems in **Alexander: Infinite Horizons**. It includes HTTP API endpoints, C++ class references, and Blueprint integration guides.

---

## HTTP API (Port 8080)

### System Status Endpoints

#### GET /api/status
**Description:** Returns overall system status and health

**Response:**
```json
{
  "status": "operational",
  "version": "0.1.0-alpha",
  "systems": {
    "persistent_universe": "online",
    "orbital_mechanics": "online",
    "farming": "online",
    "resource_gathering": "online",
    "trading_economy": "online"
  },
  "uptime": 3600.5
}
```

**C++ Implementation:**
```cpp
// In AVerificationGameMode
UFUNCTION(BlueprintCallable)
FSystemStatus GetSystemStatus();
```

---

#### GET /api/systems/{system_name}/status
**Description:** Returns detailed status for a specific system

**Parameters:**
- `system_name`: Name of the system (persistent_universe, orbital_mechanics, farming, resource_gathering, trading_economy)

**Response:**
```json
{
  "system": "farming",
  "status": "online",
  "active_farms": 15,
  "total_crops": 347,
  "performance_ms": 2.3
}
```

**C++ Implementation:**
```cpp
// In AVerificationGameMode
UFUNCTION(BlueprintCallable)
FSystemStatus GetSpecificSystemStatus(const FString& SystemName);
```

---

### Verification Endpoints

#### POST /api/verification/run
**Description:** Runs verification tests for specified systems

**Request Body:**
```json
{
  "systems": ["all"],
  "verbose": true,
  "generate_report": true
}
```

**Response:**
```json
{
  "test_id": "test_20251111_001",
  "status": "running",
  "systems": ["persistent_universe", "orbital_mechanics", "farming", "resource_gathering", "trading_economy"],
  "estimated_duration": 30
}
```

**C++ Implementation:**
```cpp
// In AVerificationGameMode
UFUNCTION(BlueprintCallable)
FTestResult RunVerificationTests(const TArray<FString>& Systems, bool bVerbose);
```

---

#### GET /api/verification/results/{test_id}
**Description:** Gets results from a specific test run

**Response:**
```json
{
  "test_id": "test_20251111_001",
  "status": "completed",
  "results": {
    "persistent_universe": {
      "passed": 45,
      "failed": 0,
      "coverage": 93
    },
    "orbital_mechanics": {
      "passed": 52,
      "failed": 0,
      "coverage": 94
    }
  },
  "overall_coverage": 94
}
```

---

### Persistent Universe API

#### GET /api/universe/state
**Description:** Gets current universe state

**Response:**
```json
{
  "origin_position": {"x": 0, "y": 0, "z": 0},
  "active_bubbles": 12,
  "total_entities": 1547,
  "save_timestamp": "2025-11-11T14:30:00Z"
}
```

**C++ Class:** [`APersistentUniverseManager`](Source/Alexander/Public/PersistentUniverseManager.h:1)

---

#### POST /api/universe/save
**Description:** Triggers universe save

**Response:**
```json
{
  "success": true,
  "save_file": "save_001.sav",
  "timestamp": "2025-11-11T14:30:00Z"
}
```

---

#### POST /api/universe/load
**Description:** Loads universe state

**Request Body:**
```json
{
  "save_file": "save_001.sav"
}
```

---

### Orbital Mechanics API

#### GET /api/orbital/systems
**Description:** Lists all star systems

**Response:**
```json
{
  "systems": [
    {
      "id": "system_001",
      "name": "Sol",
      "bodies": 9,
      "position": {"x": 0, "y": 0, "z": 0}
    }
  ],
  "total_systems": 1000
}
```

**C++ Class:** [`AOrbitalMechanicsManager`](Source/Alexander/Public/OrbitalMechanicsManager.h:1)

---

#### GET /api/orbital/bodies/{body_id}
**Description:** Gets details for a specific celestial body

**Response:**
```json
{
  "id": "body_earth",
  "name": "Earth",
  "type": "planet",
  "mass": 5.972e24,
  "radius": 6371000,
  "orbital_elements": {
    "semi_major_axis": 149600000000,
    "eccentricity": 0.0167,
    "inclination": 0
  }
}
```

---

#### POST /api/orbital/calculate_transfer
**Description:** Calculates orbital transfer between bodies

**Request Body:**
```json
{
  "from_body": "body_earth",
  "to_body": "body_mars",
  "departure_time": "2025-12-01T00:00:00Z"
}
```

**Response:**
```json
{
  "transfer_type": "hohmann",
  "delta_v": 5600,
  "duration_days": 259,
  "launch_window": "2025-12-15T00:00:00Z"
}
```

---

### Farming System API

#### GET /api/farming/farms
**Description:** Lists all active farms

**Response:**
```json
{
  "farms": [
    {
      "id": "farm_001",
      "location": "planet_earth",
      "plots": 25,
      "active_crops": 18,
      "automation_level": 0.8
    }
  ],
  "total_farms": 15
}
```

**C++ Class:** [`APlanetaryFarmingSystem`](Source/Alexander/Public/PlanetaryFarmingSystem.h:1)

---

#### GET /api/farming/crops
**Description:** Lists all crop types and status

**Response:**
```json
{
  "crops": [
    {
      "type": "wheat",
      "growth_stage": "mature",
      "time_to_harvest": 0,
      "yield_estimate": 150
    }
  ],
  "total_crops": 347
}
```

---

#### POST /api/farming/plant
**Description:** Plants new crops

**Request Body:**
```json
{
  "farm_id": "farm_001",
  "crop_type": "corn",
  "plot_id": "plot_005"
}
```

---

### Resource Gathering API

#### GET /api/resources/inventory
**Description:** Gets current resource inventory

**Response:**
```json
{
  "resources": [
    {
      "type": "iron",
      "quantity": 5000,
      "quality": 0.85,
      "location": "asteroid_belt_alpha"
    }
  ],
  "total_value": 125000
}
```

**C++ Class:** [`AResourceGatheringSystem`](Source/Alexander/Public/ResourceGatheringSystem.h:1)

---

#### GET /api/resources/asteroids
**Description:** Lists mineable asteroids

**Response:**
```json
{
  "asteroids": [
    {
      "id": "asteroid_001",
      "position": {"x": 1000, "y": 2000, "z": 3000},
      "resources": ["iron", "nickel"],
      "richness": 0.75,
      "depletion": 0.2
    }
  ]
}
```

---

#### POST /api/resources/mine
**Description:** Starts mining operation

**Request Body:**
```json
{
  "target_id": "asteroid_001",
  "resource_type": "iron",
  "duration_minutes": 30
}
```

---

### Trading Economy API

#### GET /api/economy/market
**Description:** Gets current market prices

**Response:**
```json
{
  "commodities": [
    {
      "type": "wheat",
      "buy_price": 15,
      "sell_price": 12,
      "demand": 0.8,
      "supply": 0.6
    }
  ],
  "last_updated": "2025-11-11T14:30:00Z"
}
```

**C++ Class:** [`ATradingEconomySystem`](Source/Alexander/Public/TradingEconomySystem.h:1)

---

#### GET /api/economy/factions
**Description:** Lists all factions and standings

**Response:**
```json
{
  "factions": [
    {
      "id": "terran_federation",
      "name": "Terran Federation",
      "standing": 0.75,
      "economy_type": "industrial",
      "wealth": 1000000
    }
  ]
}
```

---

#### POST /api/economy/trade
**Description:** Executes trade transaction

**Request Body:**
```json
{
  "faction_id": "terran_federation",
  "commodity": "wheat",
  "quantity": 100,
  "transaction_type": "sell"
}
```

**Response:**
```json
{
  "success": true,
  "credits_received": 1200,
  "new_standing": 0.76
}
```

---

## C++ Class Reference

### AVerificationGameMode

**Header:** [`AVerificationGameMode.h`](Source/Alexander/Public/AVerificationGameMode.h:1)

**Purpose:** Central verification and testing system

**Public Methods:**

```cpp
// Run verification tests
UFUNCTION(BlueprintCallable, Category="Verification")
FTestResult RunAllTests(bool bVerbose = false);

// Run specific system tests
UFUNCTION(BlueprintCallable, Category="Verification")
FTestResult RunSystemTests(const FString& SystemName, bool bVerbose = false);

// Generate test report
UFUNCTION(BlueprintCallable, Category="Verification")
void GenerateReport(const FTestResult& Result);

// Get system status
UFUNCTION(BlueprintCallable, Category="Verification")
FSystemStatus GetSystemStatus(const FString& SystemName);
```

**Console Commands:**
```cpp
verification.run_all_tests
verification.run_persistent_universe_tests
verification.run_orbital_mechanics_tests
verification.run_farming_tests
verification.run_resource_tests
verification.run_trading_tests
verification.generate_report
```

---

### APersistentUniverseManager

**Header:** [`PersistentUniverseManager.h`](Source/Alexander/Public/PersistentUniverseManager.h:1)

**Purpose:** Core save/load and physics management

**Public Methods:**

```cpp
// Save universe state
UFUNCTION(BlueprintCallable, Category="Persistent Universe")
bool SaveUniverse(const FString& SaveSlotName);

// Load universe state
UFUNCTION(BlueprintCallable, Category="Persistent Universe")
bool LoadUniverse(const FString& SaveSlotName);

// Get bubble status
UFUNCTION(BlueprintCallable, Category="Persistent Universe")
FBubbleInfo GetBubbleStatus(int32 BubbleID);

// Rebase origin
UFUNCTION(BlueprintCallable, Category="Persistent Universe")
void RebaseOrigin(const FVector& NewOrigin);
```

**Properties:**
```cpp
// Bubble configuration
UPROPERTY(Config)
float BubbleRadius = 10000.0f;

UPROPERTY(Config)
int32 MaxActiveBubbles = 20;

// Physics settings
UPROPERTY(Config)
bool bUseOriginRebasing = true;
```

---

### AOrbitalMechanicsManager

**Header:** [`OrbitalMechanicsManager.h`](Source/Alexander/Public/OrbitalMechanicsManager.h:1)

**Purpose:** Realistic orbital physics and procedural generation

**Public Methods:**

```cpp
// Generate star system
UFUNCTION(BlueprintCallable, Category="Orbital Mechanics")
AStarSystem* GenerateStarSystem(const FStarSystemConfig& Config);

// Calculate orbital position
UFUNCTION(BlueprintCallable, Category="Orbital Mechanics")
FVector CalculateOrbitalPosition(
    const FOrbitalElements& Elements,
    float Time
);

// Get body by ID
UFUNCTION(BlueprintCallable, Category="Orbital Mechanics")
AOrbitalBody* GetOrbitalBody(const FString& BodyID);

// Calculate transfer
UFUNCTION(BlueprintCallable, Category="Orbital Mechanics")
FHohmannTransfer CalculateHohmannTransfer(
    AOrbitalBody* FromBody,
    AOrbitalBody* ToBody,
    float DepartureTime
);
```

**Properties:**
```cpp
// Physics constants
UPROPERTY(Config)
float GravitationalConstant = 6.67430e-11f;

UPROPERTY(Config)
float PhysicsTimeStep = 0.016f;

// Generation settings
UPROPERTY(Config)
int32 MaxSystemsToGenerate = 1000;
```

---

### APlanetaryFarmingSystem

**Header:** [`PlanetaryFarmingSystem.h`](Source/Alexander/Public/PlanetaryFarmingSystem.h:1)

**Purpose:** Agriculture and base building

**Public Methods:**

```cpp
// Create farm
UFUNCTION(BlueprintCallable, Category="Farming")
AFarm* CreateFarm(const FFarmConfig& Config);

// Plant crop
UFUNCTION(BlueprintCallable, Category="Farming")
bool PlantCrop(AFarmPlot* Plot, const FCropType& CropType);

// Harvest crop
UFUNCTION(BlueprintCallable, Category="Farming")
FCropYield HarvestCrop(AFarmPlot* Plot);

// Get farm status
UFUNCTION(BlueprintCallable, Category="Farming")
FFarmStatus GetFarmStatus(AFarm* Farm);
```

**Properties:**
```cpp
// Crop definitions
UPROPERTY(Config)
TArray<FCropDefinition> CropDefinitions;

// Growth settings
UPROPERTY(Config)
float GrowthTickInterval = 1.0f;

UPROPERTY(Config)
bool bEnableAutomation = true;
```

**Crop Types (20 total):**
```cpp
// Basic crops
UPROPERTY(Config)
FCropDefinition Wheat;

UPROPERTY(Config)
FCropDefinition Corn;

UPROPERTY(Config)
FCropDefinition Rice;

// [Additional crop definitions...]
```

---

### AResourceGatheringSystem

**Header:** [`ResourceGatheringSystem.h`](Source/Alexander/Public/ResourceGatheringSystem.h:1)

**Purpose:** Mining, refining, and crafting

**Public Methods:**

```cpp
// Start mining operation
UFUNCTION(BlueprintCallable, Category="Resources")
bool StartMining(AMiningTarget* Target, const FResourceType& Resource);

// Refine resources
UFUNCTION(BlueprintCallable, Category="Resources")
bool RefineResources(const FRefiningRecipe& Recipe);

// Craft item
UFUNCTION(BlueprintCallable, Category="Resources")
bool CraftItem(const FCraftingRecipe& Recipe);

// Get inventory
UFUNCTION(BlueprintCallable, Category="Resources")
FResourceInventory GetInventory();
```

**Properties:**
```cpp
// Resource definitions
UPROPERTY(Config)
TArray<FResourceDefinition> ResourceDefinitions;

// Mining settings
UPROPERTY(Config)
float MiningTickInterval = 0.5f;

UPROPERTY(Config)
float MiningEfficiency = 1.0f;
```

**Resource Types:**
```cpp
// Common resources
UPROPERTY(Config)
FResourceDefinition Iron;

UPROPERTY(Config)
FResourceDefinition Copper;

UPROPERTY(Config)
FResourceDefinition Aluminum;

// Rare resources
UPROPERTY(Config)
FResourceDefinition Gold;

UPROPERTY(Config)
FResourceDefinition Platinum;

// [Additional resource definitions...]
```

---

### ATradingEconomySystem

**Header:** [`TradingEconomySystem.h`](Source/Alexander/Public/TradingEconomySystem.h:1)

**Purpose:** Dynamic markets and faction economy

**Public Methods:**

```cpp
// Get market prices
UFUNCTION(BlueprintCallable, Category="Trading")
FMarketPrices GetMarketPrices();

// Execute trade
UFUNCTION(BlueprintCallable, Category="Trading")
FTradeResult ExecuteTrade(const FTradeRequest& Request);

// Get faction standing
UFUNCTION(BlueprintCallable, Category="Trading")
float GetFactionStanding(const FString& FactionID);

// Generate mission
UFUNCTION(BlueprintCallable, Category="Trading")
FMission GenerateMission(const FMissionParams& Params);
```

**Properties:**
```cpp
// Faction definitions
UPROPERTY(Config)
TArray<FFactionDefinition> FactionDefinitions;

// Market settings
UPROPERTY(Config)
float PriceUpdateInterval = 60.0f;

UPROPERTY(Config)
bool bEnableDynamicPricing = true;
```

**Faction Types (12 total):**
```cpp
// Major factions
UPROPERTY(Config)
FFactionDefinition TerranFederation;

UPROPERTY(Config)
FFactionDefinition MartianCollective;

UPROPERTY(Config)
FFactionDefinition EuropaConsortium;

// [Additional faction definitions...]
```

---

## Blueprint Integration

### Exposed Functions

All major systems expose Blueprint-callable functions for designers:

```cpp
// Example: Farming System
UFUNCTION(BlueprintCallable, Category="Farming|Crops")
void PlantCrop(UPARAM(ref) FFarmPlot& Plot, FCropType CropType);

UFUNCTION(BlueprintCallable, Category="Farming|Automation")
void EnableFarmAutomation(AFarm* Farm, bool bEnabled);

// Example: Resource System
UFUNCTION(BlueprintCallable, Category="Resources|Mining")
void StartMiningOperation(AMiningTarget* Target);

UFUNCTION(BlueprintPure, Category="Resources|Inventory")
FResourceInventory GetPlayerInventory();

// Example: Trading System
UFUNCTION(BlueprintCallable, Category="Trading|Market")
void UpdateMarketPrices();

UFUNCTION(BlueprintPure, Category="Trading|Faction")
float GetPlayerFactionStanding(FString FactionID);
```

### Blueprint Events

Systems broadcast events for Blueprint handling:

```cpp
// Farming events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropGrown, FCrop, Crop);
UPROPERTY(BlueprintAssignable)
FOnCropGrown OnCropGrown;

// Resource events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceMined, FResource, Resource);
UPROPERTY(BlueprintAssignable)
FOnResourceMined OnResourceMined;

// Trading events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPriceChanged, FMarketUpdate, Update);
UPROPERTY(BlueprintAssignable)
FOnPriceChanged OnMarketPriceChanged;
```

---

## Data Structures

### Common Structures

```cpp
// Vector3 for positions
USTRUCT(BlueprintType)
struct FVector3 {
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float X, Y, Z;
};

// Resource quantity
USTRUCT(BlueprintType)
struct FResourceQuantity {
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ResourceType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Quantity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Quality;
};

// Test result
USTRUCT(BlueprintType)
struct FTestResult {
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TestID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPassed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;
};
```

---

## Error Handling

### Error Codes

```cpp
// System errors
#define ERROR_SYSTEM_NOT_INITIALIZED 1001
#define ERROR_INVALID_PARAMETERS     1002
#define ERROR_SAVE_FAILED            1003
#define ERROR_LOAD_FAILED            1004

// Resource errors
#define ERROR_INSUFFICIENT_RESOURCES 2001
#define ERROR_INVALID_RESOURCE       2002
#define ERROR_MINING_FAILED          2003

// Trading errors
#define ERROR_INVALID_TRADE          3001
#define ERROR_INSUFFICIENT_FUNDS     3002
#define ERROR_FACTION_NOT_FOUND      3003
```

### Error Response Format

```json
{
  "error": {
    "code": 1002,
    "message": "Invalid parameters provided",
    "system": "farming",
    "timestamp": "2025-11-11T14:30:00Z"
  }
}
```

---

## Performance Considerations

### API Rate Limits

- **Status endpoints:** 100 requests/minute
- **Verification endpoints:** 10 requests/minute
- **Trade endpoints:** 60 requests/minute
- **Mining endpoints:** 30 requests/minute

### Optimization Tips

1. **Batch requests** - Combine multiple operations
2. **Cache responses** - Cache static data (crop definitions, etc.)
3. **Use WebSockets** - For real-time updates
4. **Compress data** - Use gzip for large responses

---

## WebSocket API (Real-time)

### Connection
```
ws://localhost:8080/ws
```

### Events

#### farming.crop_grown
```json
{
  "event": "farming.crop_grown",
  "data": {
    "crop_id": "crop_123",
    "type": "wheat",
    "farm_id": "farm_001",
    "yield": 150
  },
  "timestamp": "2025-11-11T14:30:00Z"
}
```

#### resources.mining_complete
```json
{
  "event": "resources.mining_complete",
  "data": {
    "target_id": "asteroid_001",
    "resource": "iron",
    "quantity": 500,
    "quality": 0.85
  },
  "timestamp": "2025-11-11T14:30:00Z"
}
```

#### economy.price_update
```json
{
  "event": "economy.price_update",
  "data": {
    "commodity": "wheat",
    "old_price": 12,
    "new_price": 15,
    "demand": 0.8,
    "supply": 0.6
  },
  "timestamp": "2025-11-11T14:30:00Z"
}
```

---

## Command Line API

### Launch Options

```bash
# Run with verification
Alexander.exe -verify

# Run specific system tests
Alexander.exe -verify -system=PersistentUniverse,Farming

# Verbose output
Alexander.exe -verify -verbose

# Generate JSON report
Alexander.exe -verify -json_report
```

### Console Commands

```cpp
// Verification commands
verification.run_all_tests
verification.run_persistent_universe_tests
verification.run_orbital_mechanics_tests
verification.run_farming_tests
verification.run_resource_tests
verification.run_trading_tests
verification.generate_report

// Universe commands
universe.save
universe.load
universe.status

// Orbital commands
orbital.list_systems
orbital.get_body <body_id>
orbital.calculate_transfer <from> <to>

// Farming commands
farming.list_farms
farming.plant <farm_id> <crop_type>
farming.harvest <farm_id>

// Resource commands
resources.list_inventory
resources.mine <target_id>
resources.refine <recipe>

// Trading commands
trading.market_prices
trading.execute_trade <faction> <commodity> <quantity>
trading.faction_standing <faction_id>
```

---

## Integration Examples

### C++ Integration Example

```cpp
// Get farming system
APlanetaryFarmingSystem* FarmingSystem = 
    Cast<APlanetaryFarmingSystem>(
        UGameplayStatics::GetActorOfClass(
            GetWorld(), 
            APlanetaryFarmingSystem::StaticClass()
        )
    );

// Plant crop
FFarmPlot Plot;
FCropType Corn = FarmingSystem->GetCropDefinition("Corn");
bool bSuccess = FarmingSystem->PlantCrop(Plot, Corn);

// Get result
if (bSuccess) {
    UE_LOG(LogTemp, Log, TEXT("Crop planted successfully!"));
}
```

### Blueprint Integration Example

```cpp
// In Blueprint:
// 1. Get Farming System (Cast to APlanetaryFarmingSystem)
// 2. Call PlantCrop function
// 3. Connect Crop Type and Farm Plot
// 4. Check return value for success
```

### Python Integration Example

```python
import requests

# Get market prices
response = requests.get('http://localhost:8080/api/economy/market')
prices = response.json()

# Execute trade
trade_data = {
    'faction_id': 'terran_federation',
    'commodity': 'wheat',
    'quantity': 100,
    'transaction_type': 'sell'
}
response = requests.post('http://localhost:8080/api/economy/trade', json=trade_data)
result = response.json()
```

---

## Version History

### v0.1.0-alpha (Current)
- Initial API release
- 5 major systems implemented
- HTTP API with 20+ endpoints
- WebSocket support for real-time events
- Comprehensive error handling
- 93-95% test coverage

---

## Support & Troubleshooting

### Common Issues

**Issue:** API returns 404 Not Found
**Solution:** Ensure HTTP server is running and port 8080 is available

**Issue:** WebSocket connection fails
**Solution:** Check firewall settings and ensure WebSocket endpoint is enabled

**Issue:** Verification tests fail
**Solution:** Run with `-verbose` flag and check `Saved/VerificationReports/`

### Debug Mode

Enable debug logging:
```cpp
// In console
verification.set_log_level debug
```

### Performance Monitoring

Monitor API performance:
```bash
# Check response times
curl -w "@curl-format.txt" http://localhost:8080/api/status
```

---

*API Version:* 1.0  
*Last Updated:* November 11, 2025  
*Project Version:* v0.1.0-alpha