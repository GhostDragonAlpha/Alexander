@echo off
REM IMPROVED WORKFLOW - Get Ship Flying
REM This follows the proper Unreal workflow: Start editor once, control remotely

echo ======================================================================
echo ALEXANDER PROJECT - IMPROVED WORKFLOW
echo ======================================================================
echo.

REM Optional: If C++ code was changed, close and restart editor
set /p RESTART="Did you change C++ code? (y/N): "
if /i "%RESTART%"=="y" (
    echo [STEP 0a] Closing running editor...
    python remote_client.py --command "import unreal; unreal.get_engine().request_exit(False)"
    echo Waiting 5 seconds for shutdown...
    timeout /t 5 /nobreak >nul
    echo.

    echo [STEP 0b] Starting fresh editor...
    start "" "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
    echo Waiting 20 seconds for editor to load...
    timeout /t 20 /nobreak >nul
    echo.
) else (
    echo [STEP 1] Checking if editor is running...
    python start_editor_once.py
    echo.
)

echo [STEP 2] Testing remote connection...
python remote_client.py --test
echo.

echo [STEP 3] Taking initial screenshot...
python take_editor_screenshot.py
echo.

echo [STEP 4] Executing ship setup (Task T003)...
python remote_client.py --file "open_flight_test.py"
echo.

echo [STEP 5] Waiting for changes to apply (5 seconds)...
timeout /t 5 /nobreak
echo.

echo [STEP 6] Taking final screenshot...
python take_editor_screenshot.py
echo.

echo ======================================================================
echo WORKFLOW COMPLETE
echo ======================================================================
echo.
echo The ship should now be set up in VRTemplateMap!
echo.
echo NEXT STEPS:
echo   1. Look at the Unreal Editor window
echo   2. You should see the ship in the viewport
echo   3. Press Alt+P to test flight
echo.
echo If you need to make more changes, use:
echo   python remote_client.py --file "your_script.py"
echo.
pause
