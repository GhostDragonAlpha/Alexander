# Profiling Test Instructions

## Setup
1. Open Unreal Editor
2. Load any test level
3. Enable profiling with console command: `Profile.Start`

## Run Tests
1. Spawn multiple ships (10+)
2. Enable physics simulation
3. Fly around for 30 seconds
4. Run `Profile.Log` to see current stats

## Stop and Export
1. Run `Profile.Stop` to stop profiling
2. Report saved to: `Saved/Profiling/manual_profile.json`

## Performance Targets
- Physics Consensus:  < 5ms
- Orbital Mechanics:  < 3ms
- Flight Controller:  < 2ms
- Network:           < 1ms
- Total:             < 11ms (90fps)

## Analyze Results
Check JSON report for:
- Average frame time
- Peak memory usage
- Per-system bottlenecks
- Sample counts