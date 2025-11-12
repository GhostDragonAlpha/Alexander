@echo off
REM ============================================================================
REM ALEXANDER PROJECT - SPECIFIC TEST SUITE EXECUTION
REM ============================================================================
REM This script runs a specific test suite or category
REM
REM USAGE:
REM   run_specific_test_suite.bat [TestCategory]
REM
REM EXAMPLES:
REM   run_specific_test_suite.bat Alexander.PlanetaryFarming
REM   run_specific_test_suite.bat Alexander.Persistence
REM   run_specific_test_suite.bat Alexander.PlanetInterior
REM ============================================================================

if "%~1"=="" (
    echo Usage: run_specific_test_suite.bat [TestCategory]
    echo.
    echo Available test categories:
    echo   - Alexander.PlanetaryFarming
    echo   - Alexander.Persistence
    echo   - Alexander.PlanetInterior
    echo   - Alexander (all tests)
    echo.
    exit /b 1
)

set TEST_CATEGORY=%~1
set UE_CMD="C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set PROJECT_FILE="%~dp0Alexander.uproject"
set REPORT_DIR="%~dp0TestReports\%TEST_CATEGORY%"
set LOG_FILE="%~dp0test_execution_%TEST_CATEGORY%.log"

if not exist %REPORT_DIR% mkdir %REPORT_DIR%

echo ========================================
echo Running Test Suite: %TEST_CATEGORY%
echo ========================================
echo.

%UE_CMD% %PROJECT_FILE% ^
    -ExecCmds="Automation RunTests %TEST_CATEGORY%" ^
    -TestExit="Automation Test Queue Empty" ^
    -ReportOutputPath=%REPORT_DIR% ^
    -Log ^
    -NoSplash ^
    -Unattended ^
    -NullRHI > %LOG_FILE% 2>&1

set EXIT_CODE=%ERRORLEVEL%

echo.
echo Test suite completed with exit code: %EXIT_CODE%
echo Report: %REPORT_DIR%
echo Log: %LOG_FILE%
echo.

exit /b %EXIT_CODE%
