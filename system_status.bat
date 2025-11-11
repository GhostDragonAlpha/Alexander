@echo off
REM System status check script for Autonomous Playtesting System
REM Provides a quick overview of system health and status

echo ============================================
echo  SYSTEM STATUS CHECK
echo ============================================
echo.

REM Step 1: Check if Python is available
echo Checking Python installation...
python --version
if %errorlevel% neq 0 (
    echo ERROR: Python is not available
    pause
    exit /b 1
)
echo.

REM Step 2: Run system health check
echo Running system health check...
python system_health.py --once
echo.

REM Step 3: Check running processes
echo Checking running processes...
tasklist /FI "IMAGENAME eq python.exe" /FO TABLE
echo.

REM Step 4: Check log files
echo Checking recent log files...
if exist "capture_output\*.log" (
    echo Recent log files:
    dir "capture_output\*.log" /B /O-D
) else (
    echo No log files found
)
echo.

REM Step 5: Show service URLs
echo ============================================
echo  SERVICE STATUS
echo ============================================
echo.
echo Game Service:      http://localhost:8080
echo Analysis Service:  http://localhost:8081
echo Dashboard Service: http://localhost:8082
echo.
echo Try accessing these URLs in your browser to verify services are running.
echo.

REM Step 6: Show recent reports
echo ============================================
echo  RECENT REPORTS
echo ============================================
echo.
if exist "verification_reports\*.json" (
    echo Recent reports:
    dir "verification_reports\*.json" /B /O-D
) else (
    echo No reports found
)
echo.

echo ============================================
echo  STATUS CHECK COMPLETE
echo ============================================
echo.
echo To monitor continuously, run: python system_health.py
echo To run full validation, run: python validate_deployment.py
echo.
pause