# Alexander VR - Automated Test Execution Script (PowerShell)
# This script runs all Unreal Engine automation tests with detailed reporting

param(
    [string]$UEPath = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64",
    [switch]$Verbose,
    [switch]$QuickTest
)

# Configuration
$ProjectPath = Join-Path $PSScriptRoot "Alexander.uproject"
$UEEditor = Join-Path $UEPath "UnrealEditor-Cmd.exe"
$LogDir = Join-Path $PSScriptRoot "Saved\Logs"
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$TestLog = Join-Path $LogDir "AutomationTest_$Timestamp.log"

# Color output functions
function Write-Header {
    param([string]$Text)
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host $Text -ForegroundColor Cyan
    Write-Host "========================================`n" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Text)
    Write-Host "✅ $Text" -ForegroundColor Green
}

function Write-Error2 {
    param([string]$Text)
    Write-Host "❌ $Text" -ForegroundColor Red
}

function Write-Warning2 {
    param([string]$Text)
    Write-Host "⚠️  $Text" -ForegroundColor Yellow
}

function Write-Info {
    param([string]$Text)
    Write-Host "ℹ️  $Text" -ForegroundColor Blue
}

# Main execution
Clear-Host
Write-Header "ALEXANDER VR - AUTOMATED TEST SUITE"

# Verify Unreal Engine exists
Write-Info "Checking Unreal Engine installation..."
if (-not (Test-Path $UEEditor)) {
    Write-Error2 "Unreal Engine not found at: $UEEditor"
    Write-Warning2 "Please edit the script and set the correct -UEPath parameter"
    Write-Host "`nExpected path: $UEPath"
    Write-Host "You can run with: .\RUN_ALL_TESTS.ps1 -UEPath 'C:\Path\To\UE\Engine\Binaries\Win64'"
    exit 1
}
Write-Success "Found Unreal Engine at: $UEPath"

# Verify project file
Write-Info "Checking project file..."
if (-not (Test-Path $ProjectPath)) {
    Write-Error2 "Project file not found: $ProjectPath"
    exit 1
}
Write-Success "Found project file: Alexander.uproject"

# Create log directory
if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir | Out-Null
    Write-Info "Created logs directory: $LogDir"
}

# Display test information
Write-Host "`nTest Configuration:" -ForegroundColor Yellow
Write-Host "  Project: Alexander VR Space Simulation"
Write-Host "  Tests: 11 comprehensive system tests"
Write-Host "  Log File: $TestLog"
if ($QuickTest) {
    Write-Host "  Mode: Quick Test (essential tests only)"
} else {
    Write-Host "  Mode: Full Test Suite"
}
Write-Host ""

# Confirm execution
if (-not $Verbose) {
    Write-Host "Press any key to start testing..." -ForegroundColor Yellow
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
}

# Run tests
Write-Header "EXECUTING TESTS"

$TestCommand = if ($QuickTest) {
    "Automation RunTests Alexander.Systems"
} else {
    "Automation RunTests Alexander"
}

Write-Info "Starting test execution..."
Write-Host "Command: $TestCommand`n" -ForegroundColor DarkGray

$StartTime = Get-Date

& $UEEditor $ProjectPath `
    -ExecCmds=$TestCommand `
    -unattended `
    -nopause `
    -NullRHI `
    -log `
    -log=$TestLog

$TestResult = $LASTEXITCODE
$EndTime = Get-Date
$Duration = $EndTime - $StartTime

# Display results
Write-Header "TEST EXECUTION COMPLETE"

Write-Host "Execution Time: $($Duration.ToString('mm\:ss'))" -ForegroundColor Cyan
Write-Host "Log File: $TestLog`n"

if ($TestResult -eq 0) {
    Write-Success "All tests PASSED!"
} else {
    Write-Error2 "Some tests FAILED (Exit Code: $TestResult)"
}

# Parse log for test results
Write-Header "TEST RESULTS SUMMARY"

if (Test-Path $TestLog) {
    Write-Info "Parsing test results..."

    $LogContent = Get-Content $TestLog

    # Extract test results
    $TestLines = $LogContent | Select-String -Pattern "Alexander\.", "Test Completed", "succeeded", "failed", "tests completed"

    if ($TestLines) {
        Write-Host "`nTest Output:" -ForegroundColor Yellow
        $TestLines | ForEach-Object {
            $line = $_.Line
            if ($line -match "succeeded") {
                Write-Host "  $line" -ForegroundColor Green
            } elseif ($line -match "failed") {
                Write-Host "  $line" -ForegroundColor Red
            } else {
                Write-Host "  $line"
            }
        }
    }

    # Count results
    $PassedTests = ($LogContent | Select-String -Pattern "succeeded" -AllMatches).Count
    $FailedTests = ($LogContent | Select-String -Pattern "failed" -AllMatches).Count
    $TotalTests = $PassedTests + $FailedTests

    if ($TotalTests -gt 0) {
        Write-Host "`nFinal Summary:" -ForegroundColor Cyan
        Write-Host "  Total Tests: $TotalTests"
        Write-Success "Passed: $PassedTests"
        if ($FailedTests -gt 0) {
            Write-Error2 "Failed: $FailedTests"
        } else {
            Write-Host "  Failed: 0"
        }

        $SuccessRate = [math]::Round(($PassedTests / $TotalTests) * 100, 2)
        Write-Host "  Success Rate: $SuccessRate%`n" -ForegroundColor $(if ($SuccessRate -eq 100) { "Green" } else { "Yellow" })
    }
} else {
    Write-Warning2 "Log file not found. Tests may not have executed properly."
}

# Additional options
Write-Host "`nAdditional Commands:" -ForegroundColor Yellow
Write-Host "  View full log: notepad $TestLog"
Write-Host "  Run specific test: .\RUN_ALL_TESTS.ps1 -QuickTest"
Write-Host "  Change UE path: .\RUN_ALL_TESTS.ps1 -UEPath 'C:\Path\To\UE'"
Write-Host ""

exit $TestResult
