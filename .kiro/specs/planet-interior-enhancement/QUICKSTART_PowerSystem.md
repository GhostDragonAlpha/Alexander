# Power System Quick Start Guide

## Overview

The power system provides electricity distribution for automated farming equipment. It consists of power providers (solar panels, generators), power consumers (irrigation systems, harvesting machines), and power network nodes that connect them together.

## Components

### 1. Power Network Node (APowerNetworkNode)

The central hub that connects power providers and consumers.

**Key Features:**

- Automatically detects power providers and consumers within range
- Distributes power to connected devices
- Connects to other network nodes to form larger networks
- Updates every second to recalculate power distribution

**Default Settings:**

- Connection Range: 20 meters
- Update Interval: 1 second

**Usage:**

```cpp
// Place in level
APowerNetworkNode* Node = GetWorld()->SpawnActor<APowerNetworkNode>(Location, Rotation);

// Check network status
float AvailablePower = Node->GetTotalAvailablePower();
float PowerDemand = Node->GetTotalPowerDemand();
bool bHasPower = Node->HasSufficientPower();
```

### 2. Solar Panel (ASolarPanel)

Generates power from sunlight. Power output varies based on sun angle and time of day.

**Key Features:**

- Power output depends on sun position
- Optional sun tracking for maximum efficiency
- No fuel required
- Zero emissions

**Default Settings:**

- Max Power Output: 1000W (1 kW)
- Efficiency: 20%
- Panel Area: 2 square meters
- Min Sun Angle: 10 degrees

**Usage:**

```cpp
// Spawn solar panel
ASolarPanel* Panel = GetWorld()->SpawnActor<ASolarPanel>(Location, Rotation);

// Enable sun tracking
Panel->bSunTracking = true;
Panel->TrackingSpeed = 15.0f; // degrees per second

// Check power output
float CurrentPower = Panel->GetAvailablePower();
float PowerPercent = Panel->GetPowerOutputPercent();
```

**Power Calculation:**

- Power = MaxPower × Efficiency × AngleEfficiency
- AngleEfficiency = cos(angle between panel and sun)
- No power generated when sun is below minimum angle

### 3. Power Generator (APowerGenerator)

Generates consistent power from fuel. Provides reliable power regardless of time of day.

**Key Features:**

- Consistent power output when running
- Requires fuel to operate
- Startup time before reaching full power
- Visual and audio effects

**Default Settings:**

- Max Power Output: 5000W (5 kW)
- Fuel Type: Diesel
- Fuel Consumption: 2 liters/hour at max output
- Tank Capacity: 100 liters
- Efficiency: 30%
- Startup Time: 3 seconds

**Usage:**

```cpp
// Spawn generator
APowerGenerator* Generator = GetWorld()->SpawnActor<APowerGenerator>(Location, Rotation);

// Start generator
Generator->StartGenerator();

// Refuel
Generator->Refuel(50.0f); // Add 50 liters

// Check status
float FuelPercent = Generator->GetFuelLevelPercent();
float Runtime = Generator->GetEstimatedRuntime(); // hours
bool bHasFuel = Generator->HasSufficientFuel();

// Stop generator
Generator->StopGenerator();
```

**Fuel Types:**

- Diesel: Standard fuel, 30% efficiency
- Biofuel: Renewable, similar efficiency
- Nuclear: Very long runtime, high power
- Fusion: Future tech, highest power

### 4. Power Consumers

Devices that require power to operate.

**Irrigation System:**

- Power Requirement: 500W
- Automatically deactivates when power is lost
- Cannot activate without power

**Harvesting Machine:**

- Power Requirement: 750W (from Config.PowerConsumption)
- Stops harvesting when power is lost
- Tracks total power consumed (kWh)

## Setting Up a Power Network

### Basic Setup (Solar + Irrigation)

1. **Place Solar Panel:**

   ```cpp
   ASolarPanel* Panel = GetWorld()->SpawnActor<ASolarPanel>(
       FVector(0, 0, 100),
       FRotator::ZeroRotator
   );
   Panel->MaxPowerOutput = 1000.0f; // 1 kW
   ```

2. **Place Power Network Node:**

   ```cpp
   APowerNetworkNode* Node = GetWorld()->SpawnActor<APowerNetworkNode>(
       FVector(0, 0, 0),
       FRotator::ZeroRotator
   );
   Node->ConnectionRangeCm = 2000.0f; // 20 meters
   ```

3. **Place Irrigation System:**

   ```cpp
   AIrrigationSystem* Irrigation = GetWorld()->SpawnActor<AIrrigationSystem>(
       FVector(0, 0, 0),
       FRotator::ZeroRotator
   );
   ```

4. **Activate:**
   - Solar panel automatically generates power during daytime
   - Network node detects panel and irrigation system
   - Irrigation system receives power and can be activated

### Advanced Setup (Generator + Multiple Devices)

1. **Place Generator:**

   ```cpp
   APowerGenerator* Generator = GetWorld()->SpawnActor<APowerGenerator>(
       FVector(0, 0, 100),
       FRotator::ZeroRotator
   );
   Generator->MaxPowerOutput = 5000.0f; // 5 kW
   Generator->StartGenerator();
   ```

2. **Place Multiple Network Nodes:**

   ```cpp
   // Nodes automatically connect to each other within range
   APowerNetworkNode* Node1 = GetWorld()->SpawnActor<APowerNetworkNode>(
       FVector(0, 0, 0), FRotator::ZeroRotator
   );

   APowerNetworkNode* Node2 = GetWorld()->SpawnActor<APowerNetworkNode>(
       FVector(2000, 0, 0), FRotator::ZeroRotator
   );
   ```

3. **Place Multiple Consumers:**

   ```cpp
   // Irrigation system (500W)
   AIrrigationSystem* Irrigation = GetWorld()->SpawnActor<AIrrigationSystem>(
       FVector(0, 0, 0), FRotator::ZeroRotator
   );

   // Harvesting machine (750W)
   AHarvestingMachine* Harvester = GetWorld()->SpawnActor<AHarvestingMachine>(
       FVector(2000, 0, 0), FRotator::ZeroRotator
   );
   ```

4. **Monitor Network:**
   ```cpp
   float TotalPower = Node1->GetTotalAvailablePower(); // 5000W
   float TotalDemand = Node1->GetTotalPowerDemand(); // 1250W
   bool bSufficient = Node1->HasSufficientPower(); // true
   ```

## Power Calculation Examples

### Example 1: Solar Panel at Noon

```
Sun directly overhead (0° angle)
MaxPower = 1000W
Efficiency = 0.20
AngleEfficiency = cos(0°) = 1.0

Output = 1000 × 0.20 × 1.0 = 200W
```

### Example 2: Solar Panel at 45° Angle

```
Sun at 45° angle
MaxPower = 1000W
Efficiency = 0.20
AngleEfficiency = cos(45°) = 0.707

Output = 1000 × 0.20 × 0.707 = 141W
```

### Example 3: Generator at Full Load

```
MaxPower = 5000W
Efficiency = 0.30
Status = Running

Output = 5000 × 0.30 = 1500W
Fuel Consumption = 2 L/hour
```

### Example 4: Network with Insufficient Power

```
Available Power: 500W (1 solar panel)
Demand: 1250W (irrigation 500W + harvester 750W)

Result: Insufficient power
- Both devices receive SetPowered(false)
- Irrigation deactivates
- Harvester stops
```

## Troubleshooting

### Devices Not Receiving Power

1. **Check Network Node Range:**

   - Devices must be within ConnectionRangeCm of a network node
   - Default range is 20 meters (2000 cm)

2. **Check Power Generation:**

   - Solar panels need sunlight (check time of day)
   - Generators need fuel and must be started
   - Check GetAvailablePower() on providers

3. **Check Power Demand:**
   - Total demand must not exceed available power
   - Use GetTotalPowerDemand() to see total consumption

### Solar Panel Not Generating Power

1. **Check Sun Angle:**

   - Must be above MinSunAngle (default 10°)
   - Check GetSunDirection() and CalculateSunAngle()

2. **Check Time of Day:**

   - No power at night
   - Reduced power at dawn/dusk

3. **Check Planet Reference:**
   - Solar panel needs reference to planet for sun direction
   - Automatically finds planet with "Planet" in class name

### Generator Not Starting

1. **Check Fuel Level:**

   - Must have fuel to start
   - Use HasSufficientFuel()

2. **Check Status:**

   - May be in Starting state (wait for StartupTime)
   - Check Status enum value

3. **Refuel if Needed:**
   - Use Refuel(Amount) to add fuel
   - Check GetFuelLevelPercent()

## Blueprint Integration

### Power Provider Interface

Implement in Blueprint to create custom power sources:

```cpp
// In Blueprint, implement these functions:
- GetAvailablePower() -> float
- GetMaxPowerCapacity() -> float
- GetPowerSourceType() -> EPowerSourceType
```

### Power Consumer Interface

Implement in Blueprint to create custom power consumers:

```cpp
// In Blueprint, implement these functions:
- GetPowerRequirement() -> float
- SetPowered(bool bIsPowered)
- IsPowered() -> bool
```

## Performance Considerations

- Network nodes update every second (configurable via UpdateInterval)
- Power distribution is calculated per network, not globally
- Use fewer, larger networks rather than many small ones
- Solar panels with sun tracking have higher tick cost

## Future Enhancements

Potential additions to the power system:

1. **Battery Storage:**

   - Store excess power for nighttime use
   - Smooth out power fluctuations

2. **Power Lines:**

   - Visual representation of connections
   - Longer range connections between nodes

3. **Circuit Breakers:**

   - Automatic shutdown on overload
   - Protection for sensitive equipment

4. **Power Monitoring UI:**

   - Real-time power flow visualization
   - Historical power usage graphs

5. **Dynamic Pricing:**
   - Variable fuel costs
   - Sell excess power back to grid
