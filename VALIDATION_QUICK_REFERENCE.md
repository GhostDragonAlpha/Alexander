# Validation Quick Reference Card

**Project:** Alexander - Autonomous Baseline Validation
**Quick Access Guide for Common Validation Tasks**

---

## Quick Validation Commands

### Run Full Validation (5 minutes)
```python
# In Unreal Editor Python console
import validate_baseline
validate_baseline.run_full_validation()
```
**Output:** Console report + JSON in `Saved/ValidationReports/`

### Quick Health Check (1 minute)
```python
import validate_baseline
validate_baseline.quick_validation()
```
**Output:** 4-point status check (Missions/Stations/VFX/UI)

### Validate Individual Components
```python
import validate_baseline
report = validate_baseline.ValidationReport()

# Pick what to validate:
validate_baseline.validate_missions(report)
validate_baseline.validate_stations(report)
validate_baseline.validate_vfx(report)
validate_baseline.validate_ui(report)
validate_baseline.validate_content_structure(report)
validate_baseline.validate_json_templates(report)
validate_baseline.validate_maps(report)

report.print_report()
```

---

## Quick Testing Checklist

### Mission System (15 minutes)
- [ ] Mission board displays
- [ ] Can accept mission
- [ ] Can complete objective
- [ ] Receive rewards

### Flight & VFX (10 minutes)
- [ ] Flight controls responsive
- [ ] Engine VFX displays
- [ ] Weapon VFX triggers
- [ ] 60+ FPS maintained

### UI System (10 minutes)
- [ ] HUD displays during flight
- [ ] Mission tracker updates
- [ ] Station menus accessible
- [ ] All buttons respond

### Performance (5 minutes)
- [ ] Run PIE for 5 minutes
- [ ] Check `Saved/Profiling/` for report
- [ ] No systems >8ms
- [ ] Average FPS ≥60

---

## Expected Results

### Mission Validation
```
✓ All Source Files Present: 11/11 JSON files found with 100+ total missions
✓ Mission Directory Exists: Directory found: /Game/Data/Missions/DataAssets
✓ All Mission Categories Present: All 11 categories imported with 100+ missions
```

### Station Validation
```
✓ Station Directory Exists: Directory found: /Game/Blueprints/Stations/Generated
✓ Sufficient Stations Generated: 5/5 station blueprints available
```

### VFX Validation
```
✓ VFX Base Directory: Base directory exists
✓ VFX Configuration File: Configuration file exists
✓ VFX System Definitions: 16+ VFX systems defined
```

### UI Validation
```
✓ UI Base Directory: Base directory exists
✓ UI Configuration File: Configuration file exists
✓ UI Widget Definitions: 18+ UI widgets defined
```

---

## Performance Targets Quick Reference

| Target | VR (90 FPS) | Desktop (60 FPS) |
|--------|-------------|------------------|
| **Frame Time** | 11.11ms max | 16.67ms max |
| **FPS Average** | 90+ | 60+ |
| **System Time** | <8ms critical | <8ms critical |
| **Memory** | <4GB | <4GB |

---

## Common Issues & Quick Fixes

### "Mission directory not found"
```python
import load_mission_templates
load_mission_templates.import_all_mission_templates()
```

### "Station blueprints missing"
```python
import autonomous_station_generator
autonomous_station_generator.generate_all_stations()
```

### "VFX config missing"
```python
import autonomous_vfx_generator
autonomous_vfx_generator.create_vfx_templates()
autonomous_vfx_generator.export_vfx_config()
```

### "UI config missing"
```python
import autonomous_ui_generator
autonomous_ui_generator.create_ui_templates()
autonomous_ui_generator.export_ui_config()
```

### "Low FPS / Performance issues"
1. Check `Saved/Profiling/ProfileReport_[timestamp].json`
2. Look for systems >8ms
3. See BASELINE_PERFORMANCE_REPORT.md Section 8 for optimizations

---

## File Locations

### Validation Tools
- `Content/Python/validate_baseline.py` - Validation script
- `BASELINE_TESTING_GUIDE.md` - Full testing guide (24 tests)
- `BASELINE_COMPLETION_CHECKLIST.md` - Progress tracker (40 tasks)
- `BASELINE_PERFORMANCE_REPORT.md` - Performance template

### Reports
- `Saved/ValidationReports/baseline_validation_[timestamp].json`
- `Saved/Profiling/ProfileReport_[timestamp].json`

### Generated Content
- `Content/Data/Missions/DataAssets/` - Mission data assets
- `Content/Blueprints/Stations/Generated/` - Station blueprints
- `Content/VFX/Generated/` - VFX systems
- `Content/UI/Generated/` - UI widgets

---

## Daily Workflow

### Morning Validation (5 minutes)
```python
import validate_baseline
validate_baseline.quick_validation()
```

### After Code Changes (5 minutes)
```python
import validate_baseline
validate_baseline.run_full_validation()
```

### Weekly Performance Check (15 minutes)
1. Play in PIE for 10 minutes
2. Exit PIE
3. Check profiling report
4. Update BASELINE_PERFORMANCE_REPORT.md

### Before Milestone (1 hour)
1. Full validation: `validate_baseline.run_full_validation()`
2. All 24 manual tests (BASELINE_TESTING_GUIDE.md)
3. Performance profiling session
4. Update BASELINE_COMPLETION_CHECKLIST.md
5. Generate test report

---

## Validation Checklist (Copy/Paste)

```
[ ] Run: validate_baseline.run_full_validation()
[ ] Check: 0 critical failures
[ ] Test: Mission board displays
[ ] Test: Can complete one mission end-to-end
[ ] Test: Flight controls work
[ ] Test: VFX displays (engine, weapons)
[ ] Test: UI responsive
[ ] Profile: 5-minute PIE session
[ ] Check: Average FPS ≥60
[ ] Check: No systems >8ms
[ ] Check: Memory stable (<4GB)
```

---

## Success Criteria Summary

### Validation Passes ✓
- 0 critical failures in validation report
- All 11 mission categories present
- All required directories exist
- All configuration files created

### Testing Passes ✓
- Can accept and complete mission
- Flight controls responsive
- VFX systems operational
- UI functional and responsive

### Performance Passes ✓
- Average FPS ≥60 (≥90 for VR)
- Frame time ≤16.67ms (≤11.11ms for VR)
- 0 systems >8ms
- Memory <4GB, stable

---

## Contact Points

**Full Documentation:**
- See VALIDATION_TOOLS_SUMMARY.md for complete overview
- See BASELINE_TESTING_GUIDE.md for all 24 test cases
- See BASELINE_COMPLETION_CHECKLIST.md for task tracking
- See BASELINE_PERFORMANCE_REPORT.md for performance analysis

**Profiling System:**
- See PHASE9_FINAL_SUMMARY.md for profiling infrastructure
- See PIE_PROFILING_GUIDE.md for profiling how-to

**Generation Scripts:**
- See autonomous_baseline_generator.py for master orchestration
- See individual generator scripts in Content/Python/

---

**Version:** 1.0
**Created:** November 8, 2025
**Print This:** Keep handy during development!
