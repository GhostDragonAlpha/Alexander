@echo off
REM Launch script for Continuous Playtesting System
REM This script sets up and runs the complete playtesting framework

echo ============================================================
echo CONTINUOUS PLAYTESTING SYSTEM LAUNCHER
echo ============================================================
echo.

REM Check if Python is available
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Python is not available in PATH
    echo Please install Python and add it to your system PATH
    pause
    exit /b 1
)

REM Create necessary directories if they don't exist
if not exist "playtesting_data" mkdir playtesting_data
if not exist "playtesting_data/performance" mkdir playtesting_data\performance
if not exist "playtesting_data/issues" mkdir playtesting_data\issues
if not exist "playtesting_data/reports" mkdir playtesting_data\reports
if not exist "playtesting_data/sessions" mkdir playtesting_data\sessions
if not exist "playtesting_data/ci_results" mkdir playtesting_data\ci_results
if not exist "automation_config" mkdir automation_config
if not exist "automation_config/logs" mkdir automation_config\logs

REM Check for required configuration files
if not exist "playtesting_config.json" (
    echo ERROR: playtesting_config.json not found
    echo Please ensure the configuration file exists in the current directory
    pause
    exit /b 1
)

REM Check for required Python modules
echo Checking Python dependencies...
python -c "import requests, psutil, flask, flask_cors, PIL, cv2, numpy, mss" >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Some Python dependencies may be missing
    echo Installing required packages...
    pip install requests psutil flask flask-cors pillow opencv-python numpy mss
)

REM Parse command line arguments
set MODE=automated
set ITERATIONS=0
set CONFIG=playtesting_config.json

:parse_args
if "%~1"=="" goto :launch
if "%~1"=="/manual" set MODE=manual
if "%~1"=="/iterations" set ITERATIONS=%~2 & shift
if "%~1"=="/config" set CONFIG=%~2 & shift
shift
goto :parse_args

:launch
echo Mode: %MODE%
echo Config: %CONFIG%
if "%ITERATIONS%" NEQ "0" echo Max Iterations: %ITERATIONS%
echo.

REM Launch the continuous playtesting system
if "%MODE%"=="manual" (
    echo Starting in MANUAL mode...
    echo The game will launch and components will start, but no automated testing will run.
    echo You can watch the game and monitor the dashboard.
    echo Press Ctrl+C to stop when finished.
    echo.
    python continuous_playtesting.py --config "%CONFIG%" --manual
) else (
    echo Starting in AUTOMATED mode...
    echo The system will run continuous playtesting with automated test scenarios.
    echo Press Ctrl+C to stop when finished.
    echo.
    if "%ITERATIONS%"=="0" (
        python continuous_playtesting.py --config "%CONFIG%"
    ) else (
        python continuous_playtesting.py --config "%CONFIG%" --iterations %ITERATIONS%
    )
)

echo.
echo ============================================================
echo PLAYTESTING SESSION COMPLETE
echo ============================================================
echo.

REM Show summary of generated data
echo Generated files and data:
echo - Session data: playtesting_data\sessions\
echo - Performance data: playtesting_data\performance\
echo - Issue reports: playtesting_data\issues\
echo - Logs: automation_config\logs\
echo.

REM Check if dashboard is available
echo The dashboard should be available at: http://localhost:8082
echo.

pause