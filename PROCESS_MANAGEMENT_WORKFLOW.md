# Process Management Workflow

**Purpose**: Prevent process accumulation during multi-client testing and autonomous development

---

## Problem

During multi-client consensus testing, processes pile up:
- Multiple `UnrealEditor.exe` instances
- Multiple Python test orchestrator scripts
- Background monitoring scripts
- Old/stale processes from previous tests

**Impact**: System resource exhaustion, port conflicts, unclear test state

---

## Solution: Automated Process Lifecycle Management

### 1. Process Cleanup Utility

**File**: [cleanup_processes.py](cleanup_processes.py)

**Usage**:
```bash
# Check current process counts
python cleanup_processes.py

# Kill all Python test scripts (safe - excludes current script)
python cleanup_processes.py --python

# Kill all Unreal Editor instances
python cleanup_processes.py --unreal

# Kill everything
python cleanup_processes.py --all
```

**When to use**:
- Before starting new multi-client tests
- After completing test sessions
- When encountering "port already in use" errors
- When system becomes sluggish

### 2. Multi-Client Test with Process Management

**Recommended Pattern**:

```python
class ConsensusTestOrchestrator:
    def __init__(self):
        self.clients = []
        self.running_processes = []  # Track all spawned processes

    def launch_all_clients(self):
        """Launch clients and track their processes"""
        for config in CLIENTS:
            client = ConsensusTestClient(config)
            client.launch()
            self.clients.append(client)
            self.running_processes.append(client.process)  # Track process

        return True

    def shutdown_all_clients(self):
        """Gracefully shutdown all clients"""
        print("\nShutting down all clients...")
        for client in self.clients:
            if client.process and client.process.poll() is None:
                print(f"[Client {client.id}] Terminating process...")
                client.process.terminate()
                try:
                    client.process.wait(timeout=10)
                    print(f"[Client {client.id}] Terminated gracefully")
                except subprocess.TimeoutExpired:
                    print(f"[Client {client.id}] Force killing...")
                    client.process.kill()

        self.clients = []
        self.running_processes = []
        print("All clients shut down.")

    def run_all_tests(self):
        """Run tests with proper cleanup"""
        try:
            # Launch and test
            self.launch_all_clients()
            # ... run tests ...
        except KeyboardInterrupt:
            print("\n\nTest interrupted by user")
        except Exception as e:
            print(f"\n\nERROR: {e}")
        finally:
            # ALWAYS cleanup, even on error or Ctrl+C
            self.shutdown_all_clients()
```

**Key Points**:
1. **Track all spawned processes** in a list
2. **Use try/finally** to ensure cleanup happens
3. **Graceful shutdown** with timeout, then force kill
4. **Check process.poll()** to verify process is still running before terminating

### 3. Session Workflow

**At Session Start**:
```bash
# 1. Clean up old processes
python cleanup_processes.py --all

# 2. Verify clean slate
python cleanup_processes.py
# Should show: 0 Unreal, 0 Python

# 3. Start multi-client test
python launch_multi_client_consensus_test.py
```

**During Session**:
- Test orchestrator manages client processes
- Ctrl+C triggers graceful shutdown via `finally` block
- Processes automatically cleaned up

**At Session End**:
```bash
# 1. Verify cleanup happened
python cleanup_processes.py

# 2. If processes remain, force cleanup
python cleanup_processes.py --all
```

---

## Best Practices

### 1. Always Use Context Managers or Finally Blocks

**BAD**:
```python
def run_test():
    client.launch()
    client.spawn_ship()  # If this fails, process leaks!
    client.shutdown()
```

**GOOD**:
```python
def run_test():
    try:
        client.launch()
        client.spawn_ship()
    finally:
        client.shutdown()  # Always runs, even on error
```

### 2. Set Timeouts for Long-Running Operations

**BAD**:
```python
process.wait()  # Could hang forever
```

**GOOD**:
```python
try:
    process.wait(timeout=180)
except subprocess.TimeoutExpired:
    process.kill()
```

### 3. Track Process State

```python
# Check if process is still running
if process.poll() is None:
    # Still running
    process.terminate()
else:
    # Already exited
    print(f"Process already exited with code {process.poll()}")
```

### 4. Use Separate Console Windows (Windows)

```python
subprocess.Popen(
    cmd,
    creationflags=subprocess.CREATE_NEW_CONSOLE  # Separate window
)
```

**Benefits**:
- Each client has visible output window
- Can manually inspect client state
- Can manually close individual clients
- Easier debugging

### 5. Port Management

```python
def check_port_available(port):
    """Check if HTTP port is available"""
    try:
        response = requests.get(f"http://localhost:{port}/status", timeout=1)
        return True  # Port responding (client already running)
    except:
        return False  # Port not responding

def wait_for_port(port, timeout=180):
    """Wait for HTTP API to come online"""
    start_time = time.time()
    while time.time() - start_time < timeout:
        if check_port_available(port):
            return True
        time.sleep(3)
    return False
```

---

## Troubleshooting

### Problem: "Address already in use" (Port 8080/8081/8082)

**Cause**: Previous Unreal instance still running on that port

**Solution**:
```bash
python cleanup_processes.py --unreal
```

### Problem: Python script won't terminate

**Cause**: Waiting for subprocess that's hanging

**Solution**:
```bash
python cleanup_processes.py --python
# Or Ctrl+C then:
python cleanup_processes.py --all
```

### Problem: Too many processes running

**Cause**: Tests didn't clean up properly (exception, Ctrl+C without finally block)

**Solution**:
```bash
# Nuclear option - kill everything
python cleanup_processes.py --all

# Then restart clean
python launch_multi_client_consensus_test.py
```

### Problem: Can't kill processes

**Cause**: Process hung in uninterruptible state

**Solution** (Windows):
```bash
# Force kill by PID
taskkill /F /PID 12345

# Or kill all Unreal
taskkill /F /IM UnrealEditor.exe
```

---

## Process Lifecycle Diagram

```
START
  │
  ├─> Cleanup old processes (cleanup_processes.py --all)
  │
  ├─> Launch Client 1 (port 8080)
  │   └─> Track process in list
  │
  ├─> Launch Client 2 (port 8081)
  │   └─> Track process in list
  │
  ├─> Launch Client 3 (port 8082)
  │   └─> Track process in list
  │
  ├─> Wait for all HTTP APIs ready (180s timeout each)
  │
  ├─> Run tests
  │   ├─> Spawn ships
  │   ├─> Test position reporting
  │   └─> Test consensus validation
  │
  ├─> [Ctrl+C or Exception or Completion]
  │
  └─> FINALLY: Shutdown all clients
      ├─> For each client:
      │   ├─> Check if still running (process.poll())
      │   ├─> Terminate gracefully (process.terminate())
      │   ├─> Wait 10 seconds (process.wait(timeout=10))
      │   └─> If timeout: Force kill (process.kill())
      │
      └─> Clear process list

END
```

---

## Integration with Autonomous Workflow

### Pre-Flight Check

Add to autonomous loop iterations:

```python
def pre_flight_check():
    """Ensure clean environment before starting iteration"""
    # 1. Count processes
    result = subprocess.run(['python', 'cleanup_processes.py'], capture_output=True, text=True)

    # 2. Parse output to get counts
    if 'UnrealEditor processes:' in result.stdout:
        unreal_count = int(result.stdout.split('UnrealEditor processes:')[1].split()[0])
        python_count = int(result.stdout.split('Python processes:')[1].split()[0])

        # 3. Warn if too many
        if unreal_count > 3:
            print(f"WARNING: {unreal_count} Unreal processes running (expected 3)")
            subprocess.run(['python', 'cleanup_processes.py', '--unreal'])

        if python_count > 2:
            print(f"WARNING: {python_count} Python processes running (expected 0-2)")
            subprocess.run(['python', 'cleanup_processes.py', '--python'])
```

### Post-Flight Cleanup

```python
def post_flight_cleanup():
    """Clean up after iteration completes"""
    subprocess.run(['python', 'cleanup_processes.py', '--all'])
    print("Environment cleaned for next iteration")
```

---

## Files

### Created This Session
1. [cleanup_processes.py](cleanup_processes.py) - Process management utility
2. [PROCESS_MANAGEMENT_WORKFLOW.md](PROCESS_MANAGEMENT_WORKFLOW.md) - This file

### Modified This Session
- [launch_multi_client_consensus_test.py](launch_multi_client_consensus_test.py) - Uses proper shutdown in finally block

---

## Next Steps

1. **Test the workflow**:
   ```bash
   python cleanup_processes.py --all
   python launch_multi_client_consensus_test.py
   # Ctrl+C to test graceful shutdown
   python cleanup_processes.py  # Should show 0 processes
   ```

2. **Integrate with autonomous loop**:
   - Add pre_flight_check() at start of each iteration
   - Add post_flight_cleanup() at end of each iteration
   - Update `loop_iteration_*.py` scripts

3. **Add to documentation**:
   - Update [MASTER_TODO_SYSTEM.md](MASTER_TODO_SYSTEM.md) with process management section
   - Add to [SESSION_SUMMARY_TESTING_PHASE.md](SESSION_SUMMARY_TESTING_PHASE.md)

---

*Last Updated*: 2025-11-06
*Status*: Process management workflow implemented and documented
