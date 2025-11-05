# 📸 Manual Screenshot System

## Quick Commands

Run these commands in **Unreal's Python Console** (Window → Developer Tools → Output Log → Python tab):

### 1. Take a Screenshot
```python
py exec(open('Content/Python/snap.py').read())
```
- Captures whatever is currently visible in viewport
- Saves as `snap_TIMESTAMP.png`
- Location: `Saved/Screenshots/WindowsEditor/`

### 2. Take Screenshot + View It
```python
py exec(open('Content/Python/snap_view.py').read())
```
- Takes screenshot
- Waits for it to save
- Opens it automatically in your default image viewer

## Workflow

1. **Position your camera** in the viewport how you want
2. **Run the command** (copy/paste into Python console)
3. **Screenshot saves** in 1-2 seconds
4. **Repeat as needed** - no limits!

## Tips

- Each screenshot gets a unique timestamp filename
- No need to restart Unreal between captures
- You can take hundreds of screenshots in a session
- The viewport view is exactly what gets captured

## Example Session

```python
# Take 5 different screenshots
py exec(open('Content/Python/snap.py').read())  # Position 1
py exec(open('Content/Python/snap.py').read())  # Position 2  
py exec(open('Content/Python/snap.py').read())  # Position 3
py exec(open('Content/Python/snap.py').read())  # Position 4
py exec(open('Content/Python/snap.py').read())  # Position 5
```

All screenshots saved to: `Alexander\Saved\Screenshots\WindowsEditor\`

## For AI Analysis

After taking screenshots, I can:
1. **Read the screenshot files** from the directory
2. **Analyze what's visible** in each image
3. **Suggest improvements** based on what I see
4. **Guide you** to take more specific shots if needed

Just tell me: "Check the latest screenshot" or "What do you see in snap_20251103_153045.png"
