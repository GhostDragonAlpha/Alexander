@echo off
REM Graceful shutdown script for Autonomous Playtesting System
REM Stops all services and cleans up resources

echo ============================================
echo  STOPPING ALL SERVICES
echo ============================================
echo.

REM Step 1: Stop Python processes
echo Step 1: Stopping Python services...
taskkill /F /IM python.exe /FI "WINDOWTITLE eq Game Service*"
taskkill /F /IM python.exe /FI "WINDOWTITLE eq Analysis Service*"
taskkill /F /IM python.exe /FI "WINDOWTITLE eq Dashboard Service*"
taskkill /F /IM python.exe /FI "WINDOWTITLE eq *playtesting*"
taskkill /F /IM python.exe /FI "WINDOWTITLE eq *analysis*"
taskkill /F /IM python.exe /FI "WINDOWTITLE eq *dashboard*"
echo.

REM Step 2: Stop any remaining Python processes related to our scripts
echo Step 2: Stopping any remaining playtesting processes...
taskkill /F /FI "IMAGENAME eq python.exe" /FI "WINDOWTITLE eq *launch*"
taskkill /F /FI "IMAGENAME eq python.exe" /FI "WINDOWTITLE eq *autonomous*"
echo.

REM Step 3: Clean up temporary files
echo Step 3: Cleaning up temporary files...
if exist "screenshots_visual_playtest\*.png" (
    echo Cleaning screenshot cache...
    del /Q "screenshots_visual_playtest\*.png"
)

if exist "capture_output\*.log" (
    echo Cleaning old log files...
    del /Q "capture_output\*.log"
)
echo.

REM Step 4: Generate final health report
echo Step 4: Generating final health report...
python system_health.py --certificate
echo.

REM Step 5: Show summary
echo ============================================
echo  SHUTDOWN COMPLETE
echo ============================================
echo.
echo All services have been stopped.
echo.
echo Final reports saved to verification_reports/
echo - health_certificate.json
echo.
echo To restart the system, run: deploy_and_run.bat
echo To check status, run: system_status.bat
echo.
echo ============================================
pause