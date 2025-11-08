# Socket Crash Debugging Session - Complete Analysis

**Date:** 2025-11-07
**Status:** 🔍 ROOT CAUSE IDENTIFIED - FIX ATTEMPTED BUT STILL FAILING

---

## 🎯 INVESTIGATION SUMMARY

### Root Cause Identified
**Location:** [AutomationAPIServer.cpp:275-284](Source/Alexander/Private/AutomationAPIServer.cpp#L275-L284)
**Issue:** Socket closure race condition - server closes socket before client finishes reading response

### Windows Error Codes
- **10053 (WSAECONNABORTED)**: Software caused connection abort (local close during active I/O)
- **10054 (WSAECONNRESET)**: Connection reset by peer (TCP RST packet received)

---

## ✅ WHAT WE RULED OUT

1. ❌ **TMap/TArray reference invalidation** - Not the cause
2. ❌ **Odd vs even observer counts** - Tested with 3 (odd) and 4 (even), both crash
3. ❌ **HTTP connection pooling** - Still crashes with `Connection: close` header
4. ❌ **Data format issues** - Same payload works individually
5. ❌ **Delay-dependent race** - Tested with 0.1s, 0.5s, 1s, 2s delays
6. ❌ **Triangular geometry constraint** - The tetrahedron validation uses triangles but this doesn't cause the crash

---

## 🔬 CRASH PATTERN ANALYSIS

### Test Results

| Test | Observer Count | Crash Point | Error Code | Connection Type |
|------|---------------|-------------|------------|-----------------|
| 1 | 4 | 4th request | 10053 | Keep-alive |
| 2 | 3 | 2nd request | 10054 | Keep-alive |
| 3 | 4 | 2nd request | 10053 | Connection: close |
| 4 | 4 (post-fix) | 1st request | 10053 | Keep-alive |

### Pattern
- Crash happens on 1st, 2nd, or 4th observation submission
- **NOT deterministic** - depends on connection pool state
- **Always** a socket closure error (10053 or 10054)
- Gets progressively worse with more requests

---

## 🔍 TECHNICAL ROOT CAUSE

### The Bug (Original Code)

```cpp
// File: AutomationAPIServer.cpp, Lines 272-276 (BEFORE FIX)
Socket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);

// Close socket IMMEDIATELY after send
Socket->Close();
ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
```

**Problem:**
1. `Socket->Send()` queues data in OS send buffer and returns immediately
2. `Socket->Close()` runs **before client receives all data**
3. Client still reading → **WSAECONNABORTED** or **WSAECONNRESET**

### The Fix Attempted

```cpp
// File: AutomationAPIServer.cpp, Lines 274-284 (AFTER FIX)
Socket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);

// CRITICAL FIX: Proper socket shutdown sequence
Socket->Shutdown(ESocketShutdownMode::Write);  // Signal no more data
Socket->SetLinger(true, 2);  // 2 second linger
Socket->Close();
ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
```

**Theory:**
- `Shutdown(Write)` signals "no more data coming" without closing the socket
- `SetLinger(true, 2)` tells OS to wait up to 2 seconds for pending data to transmit
- Should allow client time to read response before socket destruction

---

## ⚠️ FIX VERIFICATION ISSUES

### Build Status
- **Runtime module built:** ✅ Alexander.exe compiled successfully (7.98 seconds)
- **AutomationAPIServer.cpp:** ✅ Compiled with socket fix
- **DLL updated:** ❓ Unknown - UnrealEditor-Alexander.dll vs Alexander.exe

### Problem
We built `Alexander.exe` (runtime game binary) but are launching with `UnrealEditor.exe` (editor binary).

**UnrealEditor.exe** loads **UnrealEditor-Alexander.dll** (NOT Alexander.exe)

**The DLL may still have the old code!**

---

## 📊 AGENT FINDINGS

### Agent 1: HTTP Server Implementation
- Uses Unreal's low-level socket API (`FSocket`, `FTcpListener`)
- Processes all requests on game thread via `FFunctionGraphTask`
- No mutex/locking mechanisms
- Closes socket immediately after each response
- Rate limited to 60 requests/second

### Agent 2: TMap/TArray Pattern Analysis
- Found 14 instances of `TMap<T, TArray<U>>` in codebase
- 2 locations use unsafe `FindOrAdd().Add()` pattern:
  - AutomationAPIServer.cpp:920 (our crash location)
  - PerformanceTestingSystem.cpp:571
- However, **crash is NOT from TMap** - agents confirmed socket issue

### Agent 3: Binary Verification
- Defensive logging (>>> markers) were NEVER compiled into running binary
- Most recent DLL timestamp: 2025-11-06 23:05:30
- Source file modified: 2025-11-06 23:17:03 (12 minutes AFTER DLL built)
- **Old binaries still being executed**

### Agent 4: Crash Pattern Analysis
- Identified socket closure as single root cause
- 10053 vs 10054 depend on exact timing of client I/O
- Connection pool state varies between test runs
- Race condition compounds with each successive request

---

## 🚨 NEXT DEBUGGING STEPS

### High Priority

1. **Verify Correct Binary is Loaded**
   ```bash
   # Check DLL timestamp
   dir "Binaries\Win64\UnrealEditor-Alexander.dll" /O-D /TW

   # Expected: After 2025-11-06 23:52:42 (socket fix compile time)
   ```

2. **Force Clean Rebuild of Editor Module**
   ```bash
   # We need UnrealEditor-Alexander.DLL, not Alexander.exe
   Build.bat AlexanderEditor Win64 Development Alexander.uproject -waitmutex
   ```

3. **Add Logging to Verify Fix is Running**
   ```cpp
   // Add BEFORE Socket->Shutdown()
   UE_LOG(LogTemp, Warning, TEXT(">>> Socket shutdown sequence BEGIN"));
   Socket->Shutdown(ESocketShutdownMode::Write);
   UE_LOG(LogTemp, Warning, TEXT(">>> Shutdown(Write) complete"));
   Socket->SetLinger(true, 2);
   UE_LOG(LogTemp, Warning, TEXT(">>> SetLinger(true, 2) complete"));
   Socket->Close();
   UE_LOG(LogTemp, Warning, TEXT(">>> Socket closed"));
   ```

4. **Test Alternative Fix: Delayed Socket Close**
   ```cpp
   // Instead of immediate close, schedule delayed close
   FTimerHandle SocketCloseTimer;
   GetWorld()->GetTimerManager().SetTimer(SocketCloseTimer, [Socket]()
   {
       Socket->Close();
       ISocketSubsystem::Get()->DestroySocket(Socket);
   }, 0.1f, false);  // 100ms delay
   ```

### Medium Priority

5. **Check if SetLinger/Shutdown APIs Work in Unreal**
   - Test if `FSocket::SetLinger()` actually calls `setsockopt(SO_LINGER)`
   - Test if `FSocket::Shutdown()` works as expected
   - May need to use raw socket APIs

6. **Alternative: Implement Keep-Alive**
   ```cpp
   // Store active connections instead of closing them
   TMap<FSocket*, float> ActiveConnections;  // Socket -> LastActivityTime

   // Reuse sockets for multiple requests
   // Only close on timeout or explicit Connection: close header
   ```

7. **Alternative: Synchronous Socket Wait**
   ```cpp
   // Wait for send buffer to drain before closing
   Socket->Wait(ESocketWaitConditions::WaitForWrite, FTimespan::FromSeconds(2));
   Socket->Close();
   ```

### Low Priority

8. **Check Unreal Engine Socket Implementation**
   - Review `FSocket::Close()` implementation in Engine source
   - Check if it's doing graceful shutdown internally
   - May need to file Unreal Engine bug

9. **Test with Raw Windows Sockets**
   ```cpp
   // Bypass Unreal's FSocket wrapper
   SOCKET RawSocket = Socket->GetNativeSocket();
   shutdown(RawSocket, SD_SEND);
   LINGER linger = {1, 2};
   setsockopt(RawSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
   closesocket(RawSocket);
   ```

---

## 💡 ALTERNATIVE SOLUTIONS

### Option 1: Single-Request Validation (WORKAROUND)
Accept all observations in one POST request:
```json
POST /validate_position_direct
{
    "target_id": 1,
    "observations": [
        {...}, {...}, {...}, {...}
    ]
}
```

**Pros:** Avoids multiple HTTP requests entirely
**Cons:** Changes API design, can't accumulate observations over time

### Option 2: External Storage (WORKAROUND)
Write observations to files instead of TMap:
```cpp
FString FilePath = FPaths::ProjectSavedDir() / TEXT("Observations") / FString::Printf(TEXT("target_%d.json"), TargetID);
FFileHelper::SaveStringToFile(JsonString, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
```

**Pros:** Avoids socket issues
**Cons:** File I/O overhead, disk writes

### Option 3: WebSockets Instead of HTTP
Use WebSockets for persistent connections:
```cpp
// Unreal has built-in WebSocket support
// Avoids connection setup/teardown overhead
// Better for real-time communication
```

**Pros:** Designed for persistent bidirectional communication
**Cons:** Requires refactoring entire API

---

## 📁 FILES MODIFIED

### C++ Source
- [Source/Alexander/Private/AutomationAPIServer.cpp](Source/Alexander/Private/AutomationAPIServer.cpp)
  - Lines 274-284: Added socket shutdown sequence
  - **Status:** Compiled in Alexander.exe, unknown in UnrealEditor-Alexander.dll

### Test Scripts
- [simple_4observer_test.py](simple_4observer_test.py) - Main test (still crashing)
- [test_3observers_triangle.py](test_3observers_triangle.py) - 3-observer test (crashed on 2nd)
- [test_http_connection_theory.py](test_http_connection_theory.py) - Connection: close test (still crashed)

---

## 🔗 RELATED FILES

- [DEFENSIVE_LOGGING_SESSION.md](DEFENSIVE_LOGGING_SESSION.md) - Initial debugging with >>> markers
- [TETRAHEDRON_VALIDATION_SESSION_SUMMARY.md](TETRAHEDRON_VALIDATION_SESSION_SUMMARY.md) - Original crash discovery
- Commit `119f9d4` - TMap atomic operations (not the actual issue)

---

## ⏱️ SESSION TIMELINE

1. **Initial Discovery:** Crash on 4th observation (ConnectionAbortedError 10053)
2. **TMap Theory:** Tested atomic operations - didn't fix it
3. **Odd-Count Theory:** Tested 3 observers - still crashed on 2nd
4. **HTTP Theory:** Tested Connection: close - still crashed on 2nd
5. **Agent Investigation:** Identified socket closure race condition
6. **Fix Attempted:** Added Shutdown() + SetLinger()
7. **Fix Verification:** **FAILED** - Still crashes on 1st request

---

## 🎯 SUCCESS CRITERIA

**✅ When debugging complete:**
1. All 4 observations submit successfully
2. No ConnectionAbortedError or ConnectionResetError
3. Full tetrahedron validation completes
4. 87.5% confidence score achieved
5. Geometric validation passes

**Current Status:** ❌ Still crashing - fix not working or not loaded

---

**NEXT ACTION: Verify new binary is actually being executed, then investigate why Shutdown/SetLinger approach didn't work.**
