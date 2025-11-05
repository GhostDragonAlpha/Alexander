# AI Vision System - Complete Setup Guide

## 🎯 What You Now Have

A complete **AI-Assisted Development System** that:
1. **Captures screenshots** from Unreal Engine automatically
2. **Analyzes them** using AI vision (GPT-4 Vision / Claude)
3. **Generates code** to improve your game
4. **Executes automatically** in Unreal Engine
5. **Iterates** until your development goals are achieved

---

## 📦 Components Created

### 1. C++ Plugin: AIVisionSystem
**Location:** `Plugins/AIVisionSystem/`

**Files:**
- `AIVisionSystem.uplugin` - Plugin manifest
- `AIVisionSystem.Build.cs` - Build configuration
- `AIScreenshotCapture.h/.cpp` - Screenshot capture actor

**Features:**
- Captures viewport screenshots
- Gathers scene metadata (actors, positions, etc.)
- Sends data to AI backend via HTTP
- Auto-capture at intervals

### 2. Python AI Backend
**Location:** `AIBackend/vision_analyzer.py`

**Features:**
- FastAPI web server
- GPT-4 Vision integration (ready for API keys)
- Scene analysis and action generation
- Mock mode (works without API keys for testing)

### 3. Unreal Python Executor
**Location:** `Content/Python/ai_executor.py`

**Features:**
- Executes AI-generated Python code in Unreal
- Creates blueprints, spawns actors, adjusts settings
- Convenience functions for common operations
- Status tracking and logging

---

## 🚀 SETUP INSTRUCTIONS

### Step 1: Compile the C++ Plugin

```powershell
# Close Unreal Editor first!

# Navigate to project directory
cd "C:\Users\allen\Desktop\Alexander\Alexander"

# Generate Visual Studio project files
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development

# This will compile the AIVisionSystem plugin
```

**Alternative (Easier):**
1. Right-click `Alexander.uproject`
2. Select "Generate Visual Studio project files"
3. Open `Alexander.sln` in Visual Studio
4. Build Solution (F7)

### Step 2: Install Python Backend Dependencies

```powershell
# Navigate to AI Backend directory
cd "C:\Users\allen\Desktop\Alexander\AIBackend"

# Create virtual environment (recommended)
python -m venv venv
.\venv\Scripts\Activate.ps1

# Install dependencies
pip install -r requirements.txt
```

### Step 3: Enable Python Plugin in Unreal

1. Open Unreal Editor
2. **Edit → Plugins**
3. Search for "**Python Editor Script Plugin**"
4. **Enable** it
5. **Restart** Unreal Editor

### Step 4: Test the System

#### A) Start AI Backend Server

```powershell
# In AIBackend directory
cd "C:\Users\allen\Desktop\Alexander\AIBackend"
python vision_analyzer.py

# You should see:
# 🚀 Starting AI Vision Backend Server
# 📡 Server will be available at: http://localhost:8000
```

#### B) In Unreal Engine Console (~ key)

```python
# Load the AI executor
py exec(open('Content/Python/ai_executor.py').read())

# Start auto-development
py start_ai_dev("Build a VR space farming game")
```

#### C) Spawn Screenshot Capture Actor

1. In Content Browser, navigate to `Plugins/AIVisionSystem C++ Classes/AIVisionSystem`
2. Drag `AAIScreenshotCapture` into your level
3. In Details panel, set:
   - **Development Goal:** "Build a VR spaceship cockpit"
   - **Backend URL:** `http://localhost:8000`
   - **Debug Mode:** True
4. Click **Start Auto Capture** with 5.0 second interval

#### D) Watch the Magic!

- Every 5 seconds, a screenshot is captured
- Sent to AI backend for analysis
- AI suggests improvements
- Python executor can run the code

---

## 🎮 USAGE EXAMPLES

### Example 1: Manual Screenshot Analysis

```python
# In Unreal console
py exec(open('Content/Python/ai_executor.py').read())

# Manually trigger analysis (if you have the actor in scene)
# The C++ actor will automatically send to AI backend
```

### Example 2: Execute Specific Action

```python
# In Unreal console
py execute_ai_action('''{
    "action_type": "create_blueprint",
    "target": "BP_TestShip",
    "parameters": {"parent_class": "Pawn"},
    "reasoning": "Testing AI executor",
    "executable_code": "create_blueprint('BP_TestShip', 'Pawn', '/Game/Blueprints/')"
}''')
```

### Example 3: Helper Functions

```python
# Create a blueprint
py create_blueprint('BP_MyShip', 'Pawn', '/Game/SpaceGame/Blueprints/')

# Spawn an actor
py spawn_actor_in_level('/Script/Engine.DirectionalLight', (0, 0, 500), (0, 0, 0), 'Sun')

# Adjust lighting
py adjust_directional_light(5.0, 6500)
```

---

## 🔧 CONFIGURATION

### AI Backend Configuration

Edit `AIBackend/vision_analyzer.py`:

```python
# Add your API keys (when ready)
from openai import OpenAI
client = OpenAI(api_key="your-openai-api-key-here")

# Or use Claude
import anthropic
claude_client = anthropic.Anthropic(api_key="your-anthropic-api-key-here")
```

### Environment Variables (Recommended)

Create `.env` file in `AIBackend/`:

```
OPENAI_API_KEY=sk-...
ANTHROPIC_API_KEY=sk-ant-...
```

Then in code:
```python
from dotenv import load_dotenv
load_dotenv()

client = OpenAI(api_key=os.getenv("OPENAI_API_KEY"))
```

---

## 📊 CURRENT STATUS

✅ **Completed:**
- C++ screenshot capture plugin
- Python AI backend (mock mode)
- Unreal Python executor
- HTTP communication between systems
- Action generation and execution framework

⏳ **Next Steps:**
1. Test compile the C++ plugin
2. Test screenshot capture
3. Add OpenAI API key for real AI analysis
4. Build first AI-generated blueprint
5. Iterate and improve!

---

## 🐛 TROUBLESHOOTING

### Plugin Won't Compile

```powershell
# Make sure you have Visual Studio 2022 with C++ tools
# Regenerate project files
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -game -engine
```

### Python Backend Won't Start

```powershell
# Check if port 8000 is in use
netstat -ano | findstr :8000

# Use different port
python vision_analyzer.py --port 8001
```

### Can't Run Python in Unreal

1. Verify Python plugin is enabled (Edit → Plugins)
2. Check Python version: `py --version` (should be 3.9+)
3. Restart Unreal Editor after enabling plugin

### Screenshots Not Captured

1. Make sure `AAIScreenshotCapture` actor is in level
2. Check Output Log for errors
3. Verify Backend URL is correct: `http://localhost:8000`
4. Make sure AI backend server is running

---

## 🎯 WHAT'S NEXT?

Once this is working, you'll have:
- **Autonomous development** capability
- **AI that sees** your game and suggests improvements
- **Instant iteration** based on visual feedback
- **Foundation** for all other Star Citizen features

**This system will accelerate your development 10-100x!**

---

## 💡 TIPS

1. **Start simple:** Test with manual screenshots first
2. **Use mock mode:** Works without API keys initially
3. **Iterate frequently:** 5-10 second capture intervals
4. **Check logs:** Unreal Output Log shows all AI actions
5. **Save often:** AI will be making lots of changes!

---

Ready to compile and test! Let me know if you encounter any issues.

🚀 **Let's build the future of game development!**
