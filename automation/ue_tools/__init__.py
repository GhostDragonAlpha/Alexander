"""
Unreal Engine 5.6 Helper Tools Automation Framework
====================================================

Python wrappers for programmatic control of UE helper programs.

Available modules:
- ue_editor: UnrealEditor.exe automation (game/server launch)
- ue_build: UnrealBuildTool and UAT automation
- ue_pak: UnrealPak.exe automation (packaging)
- ue_insights: UnrealInsights.exe automation (profiling)
- ue_patch: BuildPatchTool.exe automation (patching)
"""

__version__ = "1.0.0"
__author__ = "Alexander VR Space Simulation Team"

from pathlib import Path

# UE 5.6 installation path
UE_ENGINE_ROOT = Path(r"C:\Program Files\Epic Games\UE_5.6\Engine")
UE_BINARIES = UE_ENGINE_ROOT / "Binaries" / "Win64"
UE_BUILD_BATCH = UE_ENGINE_ROOT / "Build" / "BatchFiles"

# Project paths
PROJECT_ROOT = Path(__file__).parent.parent.parent
PROJECT_FILE = PROJECT_ROOT / "Alexander.uproject"

# Helper executables
UNREAL_EDITOR = UE_BINARIES / "UnrealEditor.exe"
UNREAL_PAK = UE_BINARIES / "UnrealPak.exe"
UNREAL_INSIGHTS = UE_BINARIES / "UnrealInsights.exe"
BUILD_PATCH_TOOL = UE_BINARIES / "BuildPatchTool.exe"
UNREAL_BUILD_TOOL = UE_BINARIES / "UnrealBuildTool.exe"

# Batch file wrappers
BUILD_BAT = UE_BUILD_BATCH / "Build.bat"
RUN_UAT_BAT = UE_BUILD_BATCH / "RunUAT.bat"

__all__ = [
    "UE_ENGINE_ROOT",
    "UE_BINARIES",
    "PROJECT_ROOT",
    "PROJECT_FILE",
    "UNREAL_EDITOR",
    "UNREAL_PAK",
    "UNREAL_INSIGHTS",
    "BUILD_PATCH_TOOL",
    "UNREAL_BUILD_TOOL",
    "BUILD_BAT",
    "RUN_UAT_BAT",
]
