@echo off
echo ========================================
echo Building Phase 7 Integration
echo ========================================

REM Set project directory
set PROJECT_DIR=%~dp0
echo Project Directory: %PROJECT_DIR%

REM Clean previous build
echo Cleaning previous build...
if exist "Binaries\Win64" (
    rmdir /s /q "Binaries\Win64"
)
if exist "Intermediate" (
    rmdir /s /q "Intermediate"
)

REM Generate project files
echo Generating project files...
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%PROJECT_DIR%Alexander.uproject" -game -rocket -progress

REM Build project
echo Building project...
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" Alexander Win64 Development -project="%PROJECT_DIR%Alexander.uproject" -game -rocket -progress

REM Check build result
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo BUILD SUCCESSFUL!
    echo ========================================
    echo.
    echo Phase 7 systems have been integrated:
    echo - Advanced AI Behaviors
    echo - Enhanced VFX System  
    echo - Audio System Manager
    echo - Tutorial & Onboarding System
    echo - Game System Coordinator
    echo.
    echo Next steps:
    echo 1. Open Alexander.uproject in Unreal Editor
    echo 2. Add Phase7SystemStartup actor to your level
    echo 3. Configure system settings as needed
    echo 4. Play in editor to test integration
    echo.
) else (
    echo.
    echo ========================================
    echo BUILD FAILED!
    echo ========================================
    echo.
    echo Please check the build log for errors.
    echo Common issues:
    echo - Missing dependencies in Build.cs
    echo - Compilation errors in new files
    echo - Missing includes
    echo.
)

pause