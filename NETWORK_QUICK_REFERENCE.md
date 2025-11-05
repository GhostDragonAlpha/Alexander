# Network Statistics - Quick Reference Guide

## Files to Modify

| File | Lines | What to Change |
|------|-------|----------------|
| NetworkReplicationManager.cpp | 3-6 | Add includes |
| NetworkReplicationManager.cpp | 532-544 | Replace UpdateNetworkStatistics() |
| NetworkManager.h | Private section | Add member variables |
| NetworkManager.cpp | 3-6 | Add includes |
| NetworkManager.cpp | 85+ | Add ping tracking to Tick() |
| NetworkManager.cpp | 412-416 | Replace GetPing() |
| NetworkManager.cpp | 418-422 | Replace GetPacketLoss() |
| ReplicationManager.h | Private section | Add compression tracking variables |
| ReplicationManager.cpp | 512-517 | Replace CompressData() |
| ReplicationManager.cpp | 519-524 | Replace DecompressData() |
| ReplicationManager.cpp | 526-540 | Replace UpdateStatistics() |
| ServerNode.cpp | 3-6 | Add NetworkManager.h include |
| ServerNode.cpp | 229-234 | Replace FindBestTargetNode() |
| NetworkPhysics.h | FInputHistoryData | Update struct definition |
| NetworkPhysics.h | Private section | Add ProcessInputOnState() declaration |
| NetworkPhysics.cpp | 3-6 | Add includes |
| NetworkPhysics.cpp | 179-195 | Replace StoreInputState() |
| NetworkPhysics.cpp | 264-280 | Replace CorrectPredictionError() |
| NetworkPhysics.cpp | End of file | Add ProcessInputOnState() implementation |

## Implementation Checklist

### Phase 1: NetworkReplicationManager.cpp
- [ ] Add PlayerState.h, NetDriver.h, NetConnection.h includes
- [ ] Replace UpdateNetworkStatistics() with real implementation
- [ ] Compile and test basic network stats

### Phase 2: NetworkManager.cpp/h
- [ ] Add member variables to header (PingHistory, LastPingUpdateTime, etc.)
- [ ] Add includes to .cpp file
- [ ] Add ping tracking code to Tick() method
- [ ] Replace GetPing() implementation
- [ ] Replace GetPacketLoss() implementation
- [ ] Compile and test ping/packet loss tracking

### Phase 3: ReplicationManager.cpp/h
- [ ] Add TotalBytesBeforeCompression, TotalBytesAfterCompression to header
- [ ] Replace CompressData() with Zlib implementation
- [ ] Replace DecompressData() with Zlib implementation
- [ ] Replace UpdateStatistics() to calculate real compression ratio
- [ ] Compile and test compression

### Phase 4: ServerNode.cpp
- [ ] Add NetworkManager.h include
- [ ] Replace FindBestTargetNode() with NetworkManager query
- [ ] Compile and test server node selection

### Phase 5: NetworkPhysics.cpp/h
- [ ] Update FInputHistoryData struct in header
- [ ] Add ProcessInputOnState() declaration to header
- [ ] Add includes to .cpp
- [ ] Replace StoreInputState() with snapshot implementation
- [ ] Replace CorrectPredictionError() with replay implementation
- [ ] Add ProcessInputOnState() method at end of file
- [ ] Compile and test input replay

## Testing Commands (Blueprint or C++)

```cpp
// Get network statistics
FAlexanderNetworkStatistics Stats = NetworkReplicationManager->GetNetworkStatistics();
UE_LOG(LogTemp, Log, TEXT("Latency: %.2f ms"), Stats.AverageLatency);
UE_LOG(LogTemp, Log, TEXT("Packet Loss: %.2f%%"), Stats.PacketLoss * 100.0f);

// Get ping
float Ping = NetworkManager->GetPing();
UE_LOG(LogTemp, Log, TEXT("Current Ping: %.2f ms"), Ping);

// Get compression stats
FReplicationStats RepStats = ReplicationManager->GetReplicationStats();
UE_LOG(LogTemp, Log, TEXT("Compression Ratio: %.2f%%"), RepStats.CompressionRatio * 100.0f);

// Get server node load
float ServerLoad = ServerNode->GetLoadPercentage();
UE_LOG(LogTemp, Log, TEXT("Server Load: %.1f%%"), ServerLoad);

// Get physics stats
float PredictionError = NetworkPhysics->GetAveragePredictionError();
int32 Reconciliations = NetworkPhysics->GetReconciliationCount();
UE_LOG(LogTemp, Log, TEXT("Avg Prediction Error: %.2f cm, Reconciliations: %d"),
    PredictionError, Reconciliations);
```

## Common Issues & Solutions

### Issue: NetDriver is null
**Solution**: Ensure you're testing in multiplayer mode (not PIE single player)

### Issue: Ping always 0
**Solution**: PlayerState->ExactPing needs time to update (~1 second after connect)

### Issue: Compression ratio is 1.0 (no compression)
**Solution**: Enable compression in ReplicationManager settings:
```cpp
ReplicationManager->bEnableCompression = true;
```

### Issue: Input replay causes jittering
**Solution**: Increase error tolerance in NetworkPhysics:
```cpp
PredictionErrorThreshold = 10.0f; // Increase from 5.0f
```

### Issue: Server can't find target node
**Solution**: Ensure NetworkManager has ServerNodes populated:
```cpp
NetworkManager->AddServerNode(FVector::ZeroVector);
```

## Performance Tuning

| Parameter | Default | Low-End | High-End |
|-----------|---------|---------|----------|
| TickRate | 30 Hz | 15 Hz | 60 Hz |
| MaxPingHistorySize | 60 | 30 | 120 |
| InputHistorySize | 100 | 50 | 200 |
| PacketLoss Alpha | 0.1 | 0.2 | 0.05 |
| Compression | BiasSpeed | BiasMemory | BiasSpeed |

## Network Profiler Integration

Add to your network HUD or debug overlay:

```cpp
// In your HUD or widget
void DrawNetworkStats()
{
    FAlexanderNetworkStatistics Stats = GetNetworkStats();

    DrawText(FString::Printf(TEXT("Ping: %.0f ms"), Stats.AverageLatency));
    DrawText(FString::Printf(TEXT("Loss: %.1f%%"), Stats.PacketLoss * 100.0f));
    DrawText(FString::Printf(TEXT("Sent: %.1f KB/s"), Stats.BytesSent / 1024.0f));
    DrawText(FString::Printf(TEXT("Recv: %.1f KB/s"), Stats.BytesReceived / 1024.0f));
    DrawText(FString::Printf(TEXT("Players: %d"), Stats.ActiveConnections));
}
```

## Bandwidth Optimization Tips

1. **Enable Compression**: Set `bEnableCompression = true` on ReplicationManager
2. **Reduce Tick Rate**: Lower tick rate for distant entities
3. **Use Dormancy**: Mark static entities as dormant
4. **Priority System**: Use Critical priority only for important entities
5. **Relevancy Radius**: Reduce radius for less important entity types

## Monitoring Dashboard Values

Create a UMG widget with these bindings:

```
Metric              | Source                                    | Format
--------------------|-------------------------------------------|--------
Ping                | NetworkManager->GetPing()                 | %.0f ms
Packet Loss         | NetworkManager->GetPacketLoss()           | %.1f %%
Upload              | NetworkReplicationManager->Statistics.BytesSent  | %.1f KB/s
Download            | NetworkReplicationManager->Statistics.BytesReceived | %.1f KB/s
Entities            | NetworkReplicationManager->Statistics.EntitiesReplicated | %d
Compression         | ReplicationManager->Stats.CompressionRatio | %.1f %%
Server Load         | ServerNode->GetLoadPercentage()           | %.0f %%
Prediction Error    | NetworkPhysics->GetAveragePredictionError() | %.1f cm
```

## Build Configuration

For shipping builds, consider disabling verbose logging:

```cpp
// In NetworkPhysics.cpp, change:
DEFINE_LOG_CATEGORY_STATIC(LogNetworkPhysics, Log, All);
// To:
DEFINE_LOG_CATEGORY_STATIC(LogNetworkPhysics, Warning, All);
```

## Estimated Implementation Time

| Phase | Task | Time |
|-------|------|------|
| 1 | NetworkReplicationManager | 15 min |
| 2 | NetworkManager | 20 min |
| 3 | ReplicationManager | 25 min |
| 4 | ServerNode | 10 min |
| 5 | NetworkPhysics | 30 min |
| Testing | All systems | 30 min |
| **Total** | | **~2 hours** |

## Support

For detailed implementation code, see: `NETWORK_STATISTICS_IMPLEMENTATION.md`
For architecture overview, see: `NETWORK_IMPLEMENTATION_SUMMARY.txt`
