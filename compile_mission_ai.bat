@echo off
REM Quick Compilation Script for Mission AI Controller
REM This script automates the compilation process

echo ================================================================================
echo MISSION AI CONTROLLER - COMPILATION SCRIPT
echo ================================================================================
echo.

REM Step 1: Check if Visual Studio is available
echo [1/4] Checking for Visual Studio...
where msbuild >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSBuild not found in PATH
    echo Please run this from a Visual Studio Developer Command Prompt
    echo Or: Open Visual Studio and build manually
    pause
    exit /b 1
)
echo Found: MSBuild

REM Step 2: Generate Visual Studio project files
echo.
echo [2/4] Generating Visual Studio project files...
if exist "Alexander.uproject" (
    "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%CD%\Alexander.uproject" -game -rocket -progress
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to generate project files
        pause
        exit /b 1
    )
    echo Successfully generated project files
) else (
    echo ERROR: Alexander.uproject not found
    echo Make sure you run this script from the project root directory
    pause
    exit /b 1
)

REM Step 3: Build the project
echo.
echo [3/4] Building C++ project (this may take several minutes)...
if exist "Alexander.sln" (
    msbuild Alexander.sln /p:Configuration="Development Editor" /p:Platform=Win64 /t:Build /m /v:minimal
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Build failed
        echo Please check the errors above
        pause
        exit /b 1
    )
    echo Build succeeded!
) else (
    echo ERROR: Alexander.sln not found
    echo Run step 2 first to generate project files
    pause
    exit /b 1
)

REM Step 4: Summary
echo.
echo [4/4] Compilation complete!
echo ================================================================================
echo SUCCESS - Mission AI Controller compiled successfully
echo ================================================================================
echo.
echo Next steps:
echo 1. Launch Unreal Editor
echo 2. Open the Alexander project
echo 3. Run the setup script:
echo    py execfile("Content/Python/setup_mission_ai_controller.py")
echo.
echo Press any key to exit...
pause >nul
