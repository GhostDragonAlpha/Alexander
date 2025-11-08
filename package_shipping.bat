@echo off
REM Package Shipping Build Script for Alexander Project
REM Creates a complete standalone shipping build

echo ========================================
echo Alexander Shipping Package Builder
echo ========================================
echo.

REM Generate timestamped build directory
for /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set mydate=%%c%%a%%b)
for /f "tokens=1-2 delims=/:" %%a in ('time /t') do (set mytime=%%a%%b)
set BUILD_DIR=C:\Users\allen\Desktop\Alexander\Builds\Shipping_%mydate%_%mytime%

echo Creating shipping build...
echo Build directory: %BUILD_DIR%
echo.
echo This may take 15-30 minutes...
echo.

"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Cook ^
  -Package ^
  -Configuration=Shipping ^
  -Platform=Win64 ^
  -StagingDirectory="%BUILD_DIR%" ^
  -Pak ^
  -Compressed ^
  -unattended ^
  -nopause

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Shipping Build Succeeded!
    echo ========================================
    echo Location: %BUILD_DIR%
    echo.
    echo Opening build directory...
    explorer "%BUILD_DIR%"
    pause
    exit /b 0
) else (
    echo.
    echo ========================================
    echo Build FAILED!
    echo ========================================
    echo Check the log files for errors.
    pause
    exit /b 1
)
