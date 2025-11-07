@echo off
REM Kill All Unreal Engine Processes
REM This script forcefully terminates all Unreal Editor and related processes

echo ========================================
echo KILLING ALL UNREAL ENGINE PROCESSES
echo ========================================
echo.

REM Kill UnrealEditor processes
echo [1/3] Killing UnrealEditor.exe...
taskkill /F /IM UnrealEditor.exe 2>nul
if %ERRORLEVEL% EQU 0 (
    echo   [SUCCESS] UnrealEditor processes terminated
) else (
    echo   [INFO] No UnrealEditor processes found
)
echo.

REM Kill UnrealEditor-Cmd processes
echo [2/3] Killing UnrealEditor-Cmd.exe...
taskkill /F /IM UnrealEditor-Cmd.exe 2>nul
if %ERRORLEVEL% EQU 0 (
    echo   [SUCCESS] UnrealEditor-Cmd processes terminated
) else (
    echo   [INFO] No UnrealEditor-Cmd processes found
)
echo.

REM Kill CrashReportClient processes
echo [3/3] Killing CrashReportClient.exe...
taskkill /F /IM CrashReportClient.exe 2>nul
if %ERRORLEVEL% EQU 0 (
    echo   [SUCCESS] CrashReportClient processes terminated
) else (
    echo   [INFO] No CrashReportClient processes found
)
echo.

REM Use PowerShell to kill any remaining Unreal processes
echo [CLEANUP] Scanning for any remaining Unreal* processes...
powershell -Command "Get-Process | Where-Object {$_.ProcessName -like '*Unreal*'} | Stop-Process -Force 2>$null"
echo   [DONE] Cleanup complete
echo.

REM Clear Live Coding lock files
echo [LOCK CLEANUP] Removing Live Coding lock files...
if exist "Intermediate\*.lock*" (
    del /F /Q Intermediate\*.lock* 2>nul
    echo   [SUCCESS] Lock files removed
) else (
    echo   [INFO] No lock files found
)
echo.

echo ========================================
echo ALL UNREAL PROCESSES TERMINATED
echo ========================================
echo.
echo You can now compile or restart the editor safely.
echo.
pause
