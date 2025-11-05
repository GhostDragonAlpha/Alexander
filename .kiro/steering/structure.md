# Project Structure & Organization

## Root Directory Layout

```
Alexander/
├── Alexander.uproject          # Main project file (UE 5.6)
├── Alexander.code-workspace    # VS Code workspace
├── Config/                     # Engine configuration
├── Content/                    # Game assets and blueprints
├── Source/                     # C++ source code
├── Plugins/                    # Custom plugins
├── Saved/                      # Generated files, logs, screenshots
├── Intermediate/               # Build artifacts
├── Binaries/                   # Compiled binaries
└── DerivedDataCache/          # Cached derived data
```

## Content Organization

```
Content/
├── AI/                        # AI-related assets and blueprints
├── Characters/                # Player and NPC assets
├── Collections/               # Asset collections
├── Effects/                   # Visual effects and particles
├── Farming/                   # Farming system assets
├── LevelPrototyping/          # Level design prototypes
├── Materials/                 # Material assets
├── Planets/                   # Planetary environments
├── Python/                    # Python scripts for AI integration
│   └── ai_executor.py         # Main AI executor script
├── SpaceShip/                 # Spaceship assets and blueprints
├── VRTemplate/                # VR-specific templates and assets
├── VRSpectator/               # VR spectator mode assets
├── Weapons/                   # Weapon systems
├── SolarSystem.umap           # Main solar system level
└── README_Development.md      # Development documentation
```

## Source Code Structure

```
Source/
├── Alexander/                 # Main game module
├── Alexander.Target.cs        # Game build target
└── AlexanderEditor.Target.cs  # Editor build target
```

## Plugin Architecture

```
Plugins/
└── AIVisionSystem/           # Custom AI vision plugin
    ├── AIVisionSystem.uplugin # Plugin manifest
    ├── Source/               # C++ source code
    ├── Binaries/            # Compiled plugin binaries
    └── Intermediate/        # Plugin build artifacts
```

## Configuration Files

- **DefaultEngine.ini**: Engine settings and rendering configuration
- **DefaultGame.ini**: Game-specific settings
- **DefaultInput.ini**: Input mappings for VR controllers
- **DefaultEditor.ini**: Editor preferences and settings

## AI Integration Files

- **run_full_auto_test.py**: Complete system integration test
- **test_ai_vision_integration.py**: AI vision system testing
- **UNREAL_COMMANDS_REFERENCE.py**: Quick reference for console commands
- **AI_VISION_SETUP.md**: Complete setup guide for AI system
- **AI_VISION_SYSTEM.md**: Technical architecture documentation

## Development Guidelines

### Asset Naming Conventions

- **Blueprints**: `BP_` prefix (e.g., `BP_PlayerShip`)
- **Materials**: `M_` prefix (e.g., `M_SpaceshipHull`)
- **Textures**: `T_` prefix (e.g., `T_MetalPlating`)
- **Static Meshes**: `SM_` prefix (e.g., `SM_CockpitPanel`)
- **VR Components**: `VR_` prefix (e.g., `BP_VR_Joystick`)

### Folder Organization

- Group related assets in themed folders (SpaceShip, Farming, etc.)
- Keep Python scripts in `Content/Python/` for easy access
- Use `Collections/` for asset management and organization
- Store prototypes in `LevelPrototyping/` before moving to final locations

### VR-Specific Structure

- All VR interactions should inherit from VR template components
- VR hand tracking assets in `VRTemplate/`
- VR spectator mode for non-VR testing in `VRSpectator/`
- Motion controller blueprints follow OpenXR standards

### AI System Integration

- Screenshot captures saved to `Saved/AICaptures/`
- AI-generated code logs in `Saved/Logs/`
- Python executor scripts in `Content/Python/`
- AI backend communication through localhost:8000 by default
