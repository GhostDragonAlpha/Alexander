# Technology Stack

## Engine & Core Technologies

- **Unreal Engine 5.6**: Primary game engine
- **C++**: Core gameplay systems and plugins
- **Blueprint Visual Scripting**: Rapid prototyping and game logic
- **Python**: AI integration, automation, and editor scripting via PythonScriptPlugin

## VR & Platform Support

- **OpenXR**: Cross-platform VR support (enabled)
- **OpenXREyeTracker**: Eye tracking integration (enabled)
- **OpenXRHandTracking**: Hand tracking for natural interactions (enabled)
- **Target Platforms**: VisionOS, Windows, Android, Linux, Mac, PlayStation, Xbox, Switch

## AI Vision System Architecture

- **FastAPI**: Python backend server for AI processing
- **OpenAI GPT-4 Vision**: Scene analysis and code generation
- **Anthropic Claude**: Alternative vision model support
- **HTTP/WebSocket**: Communication between Unreal and AI backend
- **Custom C++ Plugin**: AIVisionSystem for screenshot capture and metadata gathering

## Build System & Commands

### Project Setup

```powershell
# Generate Visual Studio project files
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="Alexander.uproject" -game -engine

# Build the project
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development
```

### AI Backend Setup

```powershell
# Navigate to AI backend (if exists)
cd AIBackend
python -m venv venv
.\venv\Scripts\Activate.ps1
pip install fastapi uvicorn openai anthropic pillow opencv-python
python vision_analyzer.py
```

### Testing Commands

```python
# In Unreal Console (` key)
py exec(open('run_full_auto_test.py').read())
py exec(open('Content/Python/ai_executor.py').read())
py start_ai_dev('Build VR spaceship cockpit')
```

## Development Workflow

1. **C++ Plugin Development**: AIVisionSystem handles screenshot capture and scene metadata
2. **Python AI Integration**: Backend processes images and generates improvement suggestions
3. **Unreal Python Executor**: Applies AI-generated changes automatically
4. **Blueprint Prototyping**: Rapid iteration for VR interactions and game mechanics
5. **VR Testing**: Continuous testing in VR headsets for comfort and usability

## Key Dependencies

- Visual Studio 2022 with C++ tools
- Python 3.9+ with AI/ML libraries
- VR headset for testing (Meta Quest, Pico, etc.)
- OpenAI or Anthropic API keys for AI vision features
