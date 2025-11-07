# Kill All Unreal Engine Processes
# PowerShell script for reliable process termination

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "KILLING ALL UNREAL ENGINE PROCESSES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Define process names to kill
$processNames = @(
    "UnrealEditor",
    "UnrealEditor-Cmd",
    "UnrealEditor-Win64-DebugGame",
    "UnrealEditor-Win64-Shipping",
    "CrashReportClient"
)

$totalKilled = 0

# Kill each process type
foreach ($procName in $processNames) {
    Write-Host "[SCANNING] Looking for $procName processes..." -ForegroundColor Yellow

    $processes = Get-Process -Name $procName -ErrorAction SilentlyContinue

    if ($processes) {
        $count = 0
        foreach ($proc in $processes) {
            Write-Host "  [FOUND] PID $($proc.Id) - $($proc.ProcessName)" -ForegroundColor White
            try {
                Stop-Process -Id $proc.Id -Force
                Write-Host "  [KILLED] PID $($proc.Id)" -ForegroundColor Green
                $count++
                $totalKilled++
            }
            catch {
                Write-Host "  [FAILED] Could not kill PID $($proc.Id): $_" -ForegroundColor Red
            }
        }
        Write-Host "  [SUCCESS] Killed $count $procName process(es)" -ForegroundColor Green
    }
    else {
        Write-Host "  [INFO] No $procName processes found" -ForegroundColor Gray
    }
    Write-Host ""
}

# Clean lock files
Write-Host "[CLEANUP] Removing lock files..." -ForegroundColor Yellow
$lockFileCount = 0

if (Test-Path "Intermediate") {
    $lockFiles = Get-ChildItem -Path "Intermediate" -Filter "*.lock*" -Recurse -ErrorAction SilentlyContinue

    if ($lockFiles) {
        foreach ($file in $lockFiles) {
            try {
                Remove-Item -Path $file.FullName -Force
                Write-Host "  [REMOVED] $($file.Name)" -ForegroundColor Green
                $lockFileCount++
            }
            catch {
                Write-Host "  [FAILED] Could not remove $($file.Name): $_" -ForegroundColor Red
            }
        }
        Write-Host "  [SUCCESS] Removed $lockFileCount lock file(s)" -ForegroundColor Green
    }
    else {
        Write-Host "  [INFO] No lock files found" -ForegroundColor Gray
    }
}
else {
    Write-Host "  [INFO] Intermediate folder not found" -ForegroundColor Gray
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "CLEANUP COMPLETE" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Summary:" -ForegroundColor White
Write-Host "  Processes killed: $totalKilled" -ForegroundColor $(if ($totalKilled -gt 0) { "Green" } else { "Gray" })
Write-Host "  Lock files removed: $lockFileCount" -ForegroundColor $(if ($lockFileCount -gt 0) { "Green" } else { "Gray" })
Write-Host ""
Write-Host "Ready to build!" -ForegroundColor Cyan
