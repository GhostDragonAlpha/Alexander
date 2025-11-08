# Defensive Logging Session - Multi-Observation Crash Debug

**Date:** 2025-11-07
**Status:** 🔍 DEBUGGING IN PROGRESS

---

## 🎯 OBJECTIVE

Identify the exact line of code where the server crashes when processing the 4th observation submission using comprehensive defensive logging.

---

## ✅ COMPLETED WORK

### 1. Implemented 8-Step Defensive Logging

Added Warning-level logging at every critical point in `HandleSubmitObservation()`:

**File:** [AutomationAPIServer.cpp:869-951](Source/Alexander/Private/AutomationAPIServer.cpp#L869-L951)

**Logging Steps:**
```cpp
>>> Step 1: Parsing JSON
>>> Step 2: Parsing observation fields
>>> Step 3: Creating measurement struct
>>> Step 4: About to store in TMap - Current TMap size: N
>>> Step 4: Stored successfully - New TMap size: N+1
>>> Step 5: Generating observation ID
>>> Step 6: Getting total observations count
>>> Step 7: Building JSON response
>>> Step 8: Creating final response
>>> AutomationAPI: HandleSubmitObservation EXIT - Success
```

### 2. Successfully Rebuilt Project

```
Build Time: 16.49 seconds
Result: Succeeded
Files Changed: AutomationAPIServer.cpp
```

### 3. Reproduced Crash Pattern (Consistent)

**Test Run:** `simple_4observer_test.py`
**Result:**
- Observer 1: ✅ Recorded
- Observer 2: ✅ Recorded
- Observer 3: ✅ Recorded
- Observer 4: ❌ `ConnectionAbortedError: [WinError 10053]`

**Delay Between Requests:** 0.1 seconds (tested with 1s, 2s - same result)

---

## 🔍 CRASH PATTERN ANALYSIS

### Key Observations

1. **Crash Point:** Always on 4th observation (not 2nd or 3rd)
2. **Consistency:** 100% reproducible
3. **Connection Type:** HTTP keep-alive (connection reused)
4. **Delay Independent:** Fails regardless of request spacing

### What This Rules Out

❌ **Simple TMap bug** - Our atomic operations fix should have solved that
❌ **Race condition** - Single-threaded test, fixed delays
❌ **Data format issue** - Same payload format for all 4 requests
❌ **Python client issue** - Curl has same problem

### What This Suggests

✅ **Accumulated state issue** - Something builds up over 3 requests
✅ **HTTP connection pooling** - Connection reused 3 times, fails on 4th
✅ **Resource threshold** - 4th operation triggers limit
✅ **Secondary container operation** - Another TMap/TArray in call chain
✅ **Memory allocation pattern** - Heap corruption after N allocations

---

## ⚠️ BINARY LOADING ISSUE

**Problem Encountered:** Could not confirm new binaries with defensive logging were loaded by running client.

**Evidence:**
- Logs do not contain `>>>` defensive markers
- Multiple client launches attempted
- Process kills executed between attempts

**Possible Causes:**
1. Client caching old DLLs
2. Multiple Unreal processes still running
3. DLL lock files preventing reload
4. Build output went to unexpected location

---

## 📊 NEXT DEBUGGING STEPS

### High Priority

1. **Confirm Binary Load**
   - Force clean rebuild: Delete Intermediate/ and Binaries/
   - Single-process test: Ensure ONLY one UnrealEditor.exe running
   - Check DLL timestamp matches build time
   - Use Process Explorer to verify loaded modules

2. **Capture Crash Moment**
   ```cpp
   // Add at line 920 (before TMap operation for 4th request)
   if (StoredObservations.FindOrAdd(TargetID).Num() == 3)
   {
       UE_LOG(LogTemp, Fatal, TEXT(">>> ABOUT TO STORE 4TH OBSERVATION - BREAKPOINT HERE"));
   }
   ```

3. **Test HTTP Connection Behavior**
   ```python
   # Force new connection for each request
   import requests
   session = requests.Session()
   session.headers.update({'Connection': 'close'})  # No keep-alive
   ```

4. **Memory Profiling**
   - Enable Unreal Memory Profiler
   - Monitor TMap allocations
   - Check for heap corruption patterns

### Medium Priority

5. **Alternative TMap Pattern**
   ```cpp
   // Try pre-allocation
   if (!StoredObservations.Contains(TargetID))
   {
       TArray<FObserverMeasurement> NewArray;
       NewArray.Reserve(10);  // Pre-allocate
       StoredObservations.Add(TargetID, MoveTemp(NewArray));
   }
   StoredObservations[TargetID].Add(Measurement);
   ```

6. **HTTP Server Thread Safety**
   ```cpp
   // Add mutex protection
   static FCriticalSection ObservationMutex;
   FScopeLock Lock(&ObservationMutex);
   StoredObservations.FindOrAdd(TargetID).Add(Measurement);
   ```

7. **Check HTTP Library Limits**
   - Review HTTP server max connections
   - Check request queue size
   - Verify no hardcoded 3-request limit

### Low Priority

8. **Workaround: Single-Request Validation**
   - Accept all 4 observations in one POST
   - Avoids multiple HTTP requests
   - Tests if issue is HTTP-specific

9. **External Logging**
   ```cpp
   // Log to file instead of UE_LOG
   FString LogPath = FPaths::ProjectSavedDir() / TEXT("observation_debug.log");
   FFileHelper::SaveStringToFile(FString::Printf(TEXT("Observation %d stored\n"), ObservationID), *LogPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
   ```

10. **Minimal Reproduction**
    - Strip down to absolute minimum
    - Remove all optional fields
    - Test with empty TMap entries

---

## 💡 HYPOTHESIS: HTTP Connection Limit

**Theory:** The HTTP server library may have a hardcoded limit of 3 requests per connection.

**Test:**
```python
# Test script
import requests

for i in range(10):
    response = requests.post(
        "http://localhost:8080/submit_observation",
        json={"observer_id": i, "target_id": 1, "direction": [1,0,0], "distance": 100, "scale_factor": 1.0},
        headers={"Connection": "close"}  # Force new connection each time
    )
    print(f"Request {i+1}: {response.status_code if response else 'FAILED'}")
```

**Expected:**
- If HTTP limit: All requests succeed with Connection:close
- If TMap issue: Still crashes on 4th request

---

## 📁 FILES MODIFIED

### C++ Source
- [Source/Alexander/Private/AutomationAPIServer.cpp](Source/Alexander/Private/AutomationAPIServer.cpp)
  - Lines 869-951: Added defensive logging (8 steps)
  - Lines 907-915: TMap atomic operations (previously committed)
  - Lines 955-973: Observer position parsing

### Test Scripts
- [simple_4observer_test.py](simple_4observer_test.py)
  - Added observer_position to observation dict
  - Changed to ASCII-safe output markers

---

## 🔗 RELATED SESSIONS

- [TETRAHEDRON_VALIDATION_SESSION_SUMMARY.md](TETRAHEDRON_VALIDATION_SESSION_SUMMARY.md) - Initial crash discovery
- Commit `119f9d4` - TMap atomic operations fix

---

## 📈 SUCCESS METRICS

**✅ When debugging complete:**
1. Defensive logs show exact crash location (Steps 1-8)
2. Root cause identified with evidence
3. Fix implemented and tested
4. All 4 observations succeed without crashes
5. Full 4-observer tetrahedron validation passes with 87.5% confidence

---

**NEXT SESSION: Load confirmed binaries with defensive logging and capture exact crash point**
