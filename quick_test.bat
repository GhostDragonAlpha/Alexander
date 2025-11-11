@echo off
REM Quick validation test script for Autonomous Playtesting System
REM Runs a fast validation of the system without full deployment

echo ============================================
echo  QUICK SYSTEM VALIDATION TEST
echo ============================================
echo.

REM Step 1: Check Python version
echo Step 1: Checking Python version...
python --version
if %errorlevel% neq 0 (
    echo ERROR: Python is not installed or not in PATH
    pause
    exit /b 1
)
echo.

REM Step 2: Validate deployment
echo Step 2: Running validation tests...
python validate_deployment.py
if %errorlevel% neq 0 (
    echo WARNING: Validation completed with issues.
) else (
    echo Validation passed!
)
echo.

REM Step 3: Run integration tests if services are running
echo Step 3: Checking if services are running...
python test_complete_system.py
if %errorlevel% neq 0 (
    echo NOTE: Some integration tests failed. Services may not be running.
    echo To start services, run: deploy_and_run.bat
)
echo.

REM Step 4: Generate health certificate
echo Step 4: Generating health certificate...
python system_health.py --certificate
echo.

REM Step 5: Show summary
echo ============================================
echo  QUICK TEST COMPLETE!
echo ============================================
echo.
echo Results saved to verification_reports/
echo - validation_certificate.json
echo - health_certificate.json
echo.
echo Next steps:
echo - If validation passed: Run deploy_and_run.bat for full deployment
echo - If issues found: Check verification_reports/ for details
echo.
echo ============================================
pause