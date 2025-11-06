@echo off
REM Autonomous Testing Launch Script
REM Compiles C++ code and runs automated flight tests
REM Uses dedicated server + multiplayer + VR hand simulation

echo ======================================================================
echo AUTONOMOUS FLIGHT TESTING SYSTEM
echo ======================================================================
echo.
echo This script will:
echo   1. Compile C++ changes (AutomationAPIServer, VRHandSimulator, etc.)
echo   2. Launch automated test suite
echo   3. Generate test reports
echo   4. Update task queue
echo.

REM ============================================================================
REM STEP 1: Compile C++ Code
REM ============================================================================

echo [STEP 1/4] Compiling C++ code...
echo.

REM Build using Unreal Build Tool
set UE_PATH=C:\Program Files\Epic Games\UE_5.6
set UBT_PATH=%UE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe
set PROJECT_PATH=%~dp0Alexander.uproject

echo Invoking UnrealBuildTool...
"%UBT_PATH%" Alexander Win64 Development "%PROJECT_PATH%" -waitmutex

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo *** BUILD FAILED ***
    echo Check build errors above
    pause
    exit /b 1
)

echo.
echo *** BUILD SUCCESSFUL ***
echo.

REM ============================================================================
REM STEP 2: Generate C++ header bindings
REM ============================================================================

echo [STEP 2/4] Generating Unreal Engine header bindings...
echo.

"%UE_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT_PATH%" -run=ResavePackages -unattended -nopause -nosplash -nullrhi

echo.
echo *** Headers generated ***
echo.

REM ============================================================================
REM STEP 3: Run Automated Tests
REM ============================================================================

echo [STEP 3/4] Running automated flight tests...
echo.

python test_flight_multiplayer.py

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo *** TESTS FAILED ***
    echo Check test output above
) else (
    echo.
    echo *** TESTS PASSED ***
)

echo.

REM ============================================================================
REM STEP 4: Generate Summary
REM ============================================================================

echo [STEP 4/4] Generating summary...
echo.

REM Check if test report exists
if exist test_report.json (
    echo Test report generated: test_report.json
    echo.

    REM Display summary
    python -c "import json; data = json.load(open('test_report.json')); print(f\"Total Tests: {data['summary']['total']}\"); print(f\"Passed: {data['summary']['passed']}\"); print(f\"Failed: {data['summary']['failed']}\"); print(f\"Pass Rate: {data['summary']['pass_rate']:.1f}%%\")"
    echo.
) else (
    echo Warning: test_report.json not found
    echo.
)

echo ======================================================================
echo AUTONOMOUS TESTING COMPLETE
echo ======================================================================
echo.
echo Next steps:
echo   1. Review test_report.json for detailed results
echo   2. Check automation_log.txt for execution logs
echo   3. View Screenshots/ folder for visual verification
echo   4. execution_queue.json has been updated with results
echo.
pause
