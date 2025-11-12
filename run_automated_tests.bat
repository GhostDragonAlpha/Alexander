@echo off
REM ============================================================================
REM ALEXANDER PROJECT - AUTOMATED TEST EXECUTION SCRIPT
REM ============================================================================
REM This script runs all automated tests using Unreal Engine's Automation Framework
REM
REM REQUIREMENTS:
REM   - Project must compile successfully (no build errors)
REM   - Unreal Engine 5.6 must be installed
REM   - UnrealEditor-Cmd.exe must be accessible
REM
REM USAGE:
REM   run_automated_tests.bat
REM
REM OUTPUT:
REM   - Test results: TestReports\index.html
REM   - Test logs: TestReports\AutomationReport.json
REM   - Console output: Saved\Logs\Alexander.log
REM ============================================================================

echo ========================================
echo ALEXANDER AUTOMATED TEST SUITE
echo ========================================
echo.

REM Set paths
set UE_CMD="C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set PROJECT_FILE="%~dp0Alexander.uproject"
set REPORT_DIR="%~dp0TestReports"
set LOG_FILE="%~dp0test_execution.log"

REM Check if UnrealEditor-Cmd exists
if not exist %UE_CMD% (
    echo [ERROR] UnrealEditor-Cmd.exe not found at:
    echo %UE_CMD%
    echo.
    echo Please verify Unreal Engine 5.6 is installed.
    exit /b 1
)

REM Check if project file exists
if not exist %PROJECT_FILE% (
    echo [ERROR] Project file not found at:
    echo %PROJECT_FILE%
    exit /b 1
)

REM Create report directory
if not exist %REPORT_DIR% mkdir %REPORT_DIR%

echo [INFO] Starting automated test execution...
echo [INFO] Engine: %UE_CMD%
echo [INFO] Project: %PROJECT_FILE%
echo [INFO] Reports: %REPORT_DIR%
echo.

REM Run all Alexander tests
echo [PHASE 1] Running all Alexander tests...
%UE_CMD% %PROJECT_FILE% ^
    -ExecCmds="Automation RunTests Alexander" ^
    -TestExit="Automation Test Queue Empty" ^
    -ReportOutputPath=%REPORT_DIR% ^
    -Log ^
    -NoSplash ^
    -Unattended ^
    -NullRHI > %LOG_FILE% 2>&1

set TEST_EXIT_CODE=%ERRORLEVEL%

echo.
echo ========================================
echo TEST EXECUTION COMPLETE
echo ========================================
echo Exit Code: %TEST_EXIT_CODE%
echo.

if %TEST_EXIT_CODE% EQU 0 (
    echo [SUCCESS] All tests completed
    echo.
    echo Test report available at:
    echo %REPORT_DIR%\index.html
) else (
    echo [WARNING] Tests completed with exit code %TEST_EXIT_CODE%
    echo This may indicate test failures or execution issues.
    echo.
    echo Check logs:
    echo - %LOG_FILE%
    echo - %REPORT_DIR%\AutomationReport.json
)

echo.
echo Press any key to open test report...
pause >nul

REM Try to open test report in browser
if exist "%REPORT_DIR%\index.html" (
    start "" "%REPORT_DIR%\index.html"
) else (
    echo [WARNING] HTML report not generated
    echo Check JSON report: %REPORT_DIR%\AutomationReport.json
)

exit /b %TEST_EXIT_CODE%
