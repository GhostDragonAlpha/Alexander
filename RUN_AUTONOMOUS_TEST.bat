@echo off
REM Complete Autonomous Flight Testing Workflow
REM Step 1: Build project
REM Step 2: Launch standalone game
REM Step 3: Run automated tests
REM Step 4: Collect results
REM Step 5: Shutdown

echo ======================================================================
echo AUTONOMOUS FLIGHT TESTING - COMPLETE WORKFLOW
echo ======================================================================
echo.

REM ============================================================================
REM STEP 1: Build Project (if needed)
REM ============================================================================

echo [STEP 1/5] Checking if build is needed...
echo.

REM Check if built recently (within last hour)
set BUILD_NEEDED=0

if not exist "Binaries\Win64\Alexander.exe" (
    echo [INFO] No existing build found
    set BUILD_NEEDED=1
) else (
    echo [INFO] Existing build found
    echo [INFO] Skipping rebuild - use Build.bat if code changed
)

if %BUILD_NEEDED%==1 (
    echo.
    echo [INFO] Building project...
    call "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "Alexander.uproject" -waitmutex

    if %ERRORLEVEL% NEQ 0 (
        echo [FAIL] Build failed
        pause
        exit /b 1
    )
    echo [OK] Build complete
)

echo.

REM ============================================================================
REM STEP 2: Launch Standalone Game
REM ============================================================================

echo [STEP 2/5] Launching standalone game...
echo.

set GAME_EXE=C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=%~dp0Alexander.uproject
set MAP=VRTemplateMap

echo Starting game: %MAP%
echo.

REM Launch game in background
start "Alexander_AutoTest" /MIN "%GAME_EXE%" "%PROJECT%" %MAP% -game -log -windowed -ResX=800 -ResY=600

echo [INFO] Game launching...
echo [INFO] Waiting 15 seconds for game to load...
timeout /t 15 /nobreak >nul

echo [OK] Game should be loaded
echo.

REM ============================================================================
REM STEP 3: Run Automated Tests
REM ============================================================================

echo [STEP 3/5] Running autonomous flight tests...
echo.

REM Run the test script
echo | python test_standalone_flight.py

if %ERRORLEVEL% NEQ 0 (
    echo [WARN] Test script returned error code: %ERRORLEVEL%
) else (
    echo [OK] Test script completed
)

echo.

REM ============================================================================
REM STEP 4: Collect Results
REM ============================================================================

echo [STEP 4/5] Collecting results...
echo.

REM Wait a moment for final logs
timeout /t 2 /nobreak >nul

echo [INFO] Test execution complete
echo [INFO] Check game log for detailed results
echo.

REM ============================================================================
REM STEP 5: Shutdown Game
REM ============================================================================

echo [STEP 5/5] Shutting down game...
echo.

REM Kill the game process
taskkill /FI "WINDOWTITLE eq Alexander_AutoTest*" /F >nul 2>&1
timeout /t 2 /nobreak >nul
taskkill /IM UnrealEditor.exe /F >nul 2>&1

echo [OK] Game shutdown complete
echo.

REM ============================================================================
REM FINAL SUMMARY
REM ============================================================================

echo ======================================================================
echo AUTONOMOUS TEST COMPLETE
echo ======================================================================
echo.
echo Results:
echo   - Check console output above
echo   - Look for "[SUCCESS] SHIP IS FLYING!" message
echo   - Game log saved to: Saved\Logs\Alexander.log
echo.
echo Next steps:
echo   - Review logs for detailed results
echo   - Re-run anytime with: RUN_AUTONOMOUS_TEST.bat
echo   - For manual testing: launch_standalone_game.bat
echo.
pause
