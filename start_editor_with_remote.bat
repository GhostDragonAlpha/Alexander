@echo off
REM Start Unreal Editor with Python Remote Execution enabled
REM This keeps the editor running so we can send commands to it

echo Starting Unreal Editor with Python Remote Execution...
echo.
echo The editor will stay open.
echo You can send Python commands to it using remote_client.py
echo.

start "" "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" VRTemplateMap -game -log

echo.
echo Editor is starting...
echo Wait for it to fully load, then run: python remote_client.py
echo.
pause
