# Quick Start - Get Ship Flying NOW

## Step 1: Enable Remote Execution (One-Time Setup)

**If you haven't done this yet:**

1. Open Unreal Editor manually
2. Edit → Project Settings
3. Search for "Python"
4. Under Python section:
   - ✓ Enable Remote Execution
   - Multicast Group: `239.0.0.1`
   - Multicast Port: `6766`
5. Click "Set as Default"
6. Restart editor

---

## Step 2: Run the Automated Workflow

**Option A: Full automation (Windows)**
```bash
EXECUTE_WORKFLOW.bat
```

**Option B: Manual steps (any platform)**
```bash
# 1. Start editor (once)
python start_editor_once.py

# 2. Test connection
python remote_client.py --test

# 3. Get ship flying
python remote_client.py --file "open_flight_test.py"

# 4. Take screenshot to verify
python take_editor_screenshot.py
```

---

## Step 3: Test Flight

1. Look at the Unreal Editor window
2. Press **Alt+P** (Play In Editor)
3. You should spawn in the spaceship!

---

## If Remote Execution Doesn't Work

**Manual Method:**

1. Open Unreal Editor
2. Open the **Output Log** (Window → Developer Tools → Output Log)
3. At the bottom, there's a command input box
4. Type this command:
   ```python
   py exec(open('C:/Users/allen/Desktop/Alexander/Alexander/open_flight_test.py').read())
   ```
5. Press Enter
6. Watch the Output Log for results
7. Press Alt+P to test flight

---

## Verify It Worked

**You'll know it worked if:**
- ✓ Output Log shows "TASK T003 COMPLETE"
- ✓ You see the ship in the viewport
- ✓ Pressing Alt+P spawns you in the ship
- ✓ You can move/fly the ship

**If something failed:**
- Check the Output Log for error messages
- Look at the screenshots in the Screenshots/ folder
- Read [IMPROVED_WORKFLOW.md](IMPROVED_WORKFLOW.md) for troubleshooting

---

## Next Steps After Ship is Flying

Update [execution_queue.json](execution_queue.json):
- Mark T003 as "completed"
- Move to T004 (performance check)
- Follow the task list systematically

Use remote execution for all future changes:
```bash
python remote_client.py --file "your_next_task.py"
```

**Editor stays running! No more constant restarts!**
