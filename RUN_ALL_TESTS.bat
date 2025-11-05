@echo off
REM Alexander VR - Automated Test Execution Script
REM This script runs all Unreal Engine automation tests

echo ========================================
echo ALEXANDER VR - TEST EXECUTION
echo ========================================
echo.

REM Set Unreal Engine path (adjust if needed)
set UE_PATH=C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64
set PROJECT_PATH=%~dp0Alexander.uproject

REM Check if Unreal Engine exists
if not exist "%UE_PATH%\UnrealEditor-Cmd.exe" (
    echo ERROR: Unreal Engine not found at: %UE_PATH%
    echo Please edit this script and set the correct UE_PATH
    pause
    exit /b 1
)

echo Found Unreal Engine at: %UE_PATH%
echo Project: %PROJECT_PATH%
echo.

REM Create logs directory if it doesn't exist
if not exist "%~dp0Saved\Logs" mkdir "%~dp0Saved\Logs"

set TEST_LOG=%~dp0Saved\Logs\AutomationTest_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log
set TEST_LOG=%TEST_LOG: =0%

echo ========================================
echo RUNNING AUTOMATED TESTS
echo ========================================
echo.
echo This will run all Alexander project tests.
echo Test results will be saved to:
echo %TEST_LOG%
echo.
echo Press any key to start testing...
pause >nul

echo.
echo [1/3] Running all Alexander tests...
echo.

"%UE_PATH%\UnrealEditor-Cmd.exe" ^
  "%PROJECT_PATH%" ^
  -ExecCmds="Automation RunTests Alexander" ^
  -unattended ^
  -nopause ^
  -NullRHI ^
  -log ^
  -log="%TEST_LOG%"

set TEST_RESULT=%ERRORLEVEL%

echo.
echo ========================================
echo TEST EXECUTION COMPLETE
echo ========================================
echo.

if %TEST_RESULT% EQU 0 (
    echo Result: SUCCESS - All tests passed!
    echo.
    echo Check the log file for detailed results:
    echo %TEST_LOG%
) else (
    echo Result: FAILED - Some tests failed.
    echo Error Code: %TEST_RESULT%
    echo.
    echo Check the log file for details:
    echo %TEST_LOG%
)

echo.
echo ========================================
echo QUICK TEST SUMMARY
echo ========================================
echo.
echo Searching for test results in log...
findstr /C:"Automation Test Queue" /C:"tests completed" /C:"succeeded" /C:"failed" "%TEST_LOG%"

echo.
echo ========================================
echo Press any key to exit...
pause >nul

exit /b %TEST_RESULT%
