@echo off
REM Launch standalone game with Python Remote Execution
REM This runs the actual game (not editor) for autonomous testing

echo ======================================================================
echo LAUNCHING STANDALONE GAME FOR AUTONOMOUS TESTING
echo ======================================================================
echo.

set GAME_EXE=C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject
set MAP=VRTemplateMap

echo Game: %GAME_EXE%
echo Project: %PROJECT%
echo Map: %MAP%
echo.
echo Starting game in standalone mode...
echo Python Remote Execution will be available on port 6766
echo.

REM Launch in standalone game mode (not editor)
REM -game = Run as game, not editor
REM -log = Show log window
REM -windowed = Run in window (not fullscreen)
REM -ResX/ResY = Window size
start "Alexander Game" "%GAME_EXE%" "%PROJECT%" %MAP% -game -log -windowed -ResX=1280 -ResY=720

echo.
echo ======================================================================
echo GAME LAUNCHED
echo ======================================================================
echo.
echo Wait for game to fully load (watch log window)
echo When you see "LogInit: Display: Game Engine Initialized"
echo Then run: python test_standalone_flight.py
echo.
echo To shutdown: Close the game window or run taskkill
echo.
pause
