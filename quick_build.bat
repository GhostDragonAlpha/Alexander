@echo off
REM Quick Build Script for Alexander Project
REM Compiles C++ code using UnrealFrontend

echo ========================================
echo Alexander Quick Build (Development)
echo ========================================
echo.

"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Configuration=Development ^
  -Platform=Win64 ^
  -unattended ^
  -nopause

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build Succeeded!
    echo ========================================
    exit /b 0
) else (
    echo.
    echo ========================================
    echo Build FAILED!
    echo ========================================
    pause
    exit /b 1
)
