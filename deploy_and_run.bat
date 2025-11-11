@echo off
REM Complete deployment and start script for Autonomous Playtesting System
REM This script deploys the system and starts all services

echo ============================================
echo  AUTONOMOUS PLAYTESTING SYSTEM DEPLOYMENT
echo ============================================
echo.

REM Step 1: Run deployment script
echo Step 1: Running system deployment...
python deploy_playtesting_system.py
if %errorlevel% neq 0 (
    echo ERROR: Deployment failed. Please check the errors above.
    pause
    exit /b 1
)
echo.

REM Step 2: Validate deployment
echo Step 2: Validating deployment...
python validate_deployment.py
if %errorlevel% neq 0 (
    echo WARNING: Validation completed with issues. Check the report.
) else (
    echo Deployment validation passed!
)
echo.

REM Step 3: Start all services
echo Step 3: Starting all services...
echo Starting Game Service (Port 8080)...
start "Game Service" python launch_game_tracked.py

timeout /t 5

echo Starting Analysis Service (Port 8081)...
start "Analysis Service" python launch_autonomous_testing.py

timeout /t 5

echo Starting Dashboard Service (Port 8082)...
start "Dashboard Service" python playtesting_dashboard.py

timeout /t 10

REM Step 4: Check system health
echo.
echo Step 4: Checking system health...
python system_health.py --once

REM Step 5: Show status
echo.
echo ============================================
echo  DEPLOYMENT COMPLETE!
echo ============================================
echo.
echo Services started:
echo - Game Service: http://localhost:8080
echo - Analysis Service: http://localhost:8081
echo - Dashboard Service: http://localhost:8082
echo.
echo To monitor system health, run: python system_health.py
echo To run integration tests, run: python test_complete_system.py
echo To stop all services, run: stop_all.bat
echo.
echo ============================================
pause