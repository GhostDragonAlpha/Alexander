# Quick Reference Guide

## Automation Framework Commands

### Building the Editor
```bash
python automation_client.py build-editor
```
Builds the Unreal Editor for the Alexander project.

### Building for Release
```bash
python automation_client.py build-shipping
```
Creates an optimized shipping build (no debug symbols, maximum performance).

### Creating Distribution Paks
```bash
python automation_client.py create-pak
```
Packages game content into .pak files for distribution.

### Running Profiling
```bash
python automation_client.py profile
```
Runs performance profiling session and generates reports.

### Nightly Build
```bash
python automation_client.py nightly-build
```
Full automated build pipeline: clean build + tests + profiling + pak creation.

## UE 5.6 TMap Fix Pattern

Unreal Engine 5.6 requires wrapper structs for UPROPERTY TMap values. Here's how to fix the compilation errors:

### Step 1: Create Wrapper Struct (in .h file)

**Before:**
```cpp
UPROPERTY()
TMap<FString, float> MyData;
```

**After:**
```cpp
// Add wrapper struct ABOVE the class that uses it
USTRUCT(BlueprintType)
struct FMyDataEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Key;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Value;

    FMyDataEntry() : Value(0.0f) {}
    FMyDataEntry(const FString& InKey, float InVal) : Key(InKey), Value(InVal) {}
};

// In your class:
UPROPERTY(BlueprintReadWrite, EditAnywhere)
TArray<FMyDataEntry> MyData;
```

### Step 2: Update .cpp File Logic

**Before:**
```cpp
MyData.Add(TEXT("Speed"), 100.0f);
float Speed = MyData[TEXT("Speed")];

for (const auto& Pair : MyData)
{
    FString Key = Pair.Key;
    float Value = Pair.Value;
}
```

**After:**
```cpp
MyData.Add(FMyDataEntry(TEXT("Speed"), 100.0f));

// Finding values:
float Speed = 0.0f;
for (const FMyDataEntry& Entry : MyData)
{
    if (Entry.Key == TEXT("Speed"))
    {
        Speed = Entry.Value;
        break;
    }
}

// Iterating:
for (const FMyDataEntry& Entry : MyData)
{
    FString Key = Entry.Key;
    float Value = Entry.Value;
}
```

### Common TMap Patterns

#### Pattern 1: String to Float
```cpp
USTRUCT(BlueprintType)
struct FStringFloatEntry
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Key;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Value;
};
```

#### Pattern 2: String to Integer
```cpp
USTRUCT(BlueprintType)
struct FStringIntEntry
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Key;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Value;
};
```

#### Pattern 3: Enum to Struct
```cpp
USTRUCT(BlueprintType)
struct FEnumToDataEntry
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TEnumAsByte<EMyEnum> Key;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FMyStruct Value;
};
```

### Helper Function Pattern

Add this helper to make lookups easier:

```cpp
// In .h file:
float GetFloatValue(const TArray<FStringFloatEntry>& Array, const FString& Key, float DefaultValue = 0.0f) const;

// In .cpp file:
float AMyClass::GetFloatValue(const TArray<FStringFloatEntry>& Array, const FString& Key, float DefaultValue) const
{
    for (const FStringFloatEntry& Entry : Array)
    {
        if (Entry.Key == Key)
        {
            return Entry.Value;
        }
    }
    return DefaultValue;
}

// Usage:
float Speed = GetFloatValue(MyData, TEXT("Speed"), 100.0f);
```

## Troubleshooting

### Build Error: "TMap UPROPERTY not supported"
**Problem:** Using TMap as UPROPERTY in UE 5.6+

**Solution:** Convert to wrapper struct pattern (see above)

### Build Error: "Cannot find UnrealBuildTool"
**Problem:** UE_ROOT environment variable not set or incorrect

**Solution:**
```bash
# Check current setting:
echo $env:UE_ROOT

# Set to your UE installation (in PowerShell):
$env:UE_ROOT = "C:\Program Files\Epic Games\UE_5.6"

# Or add to system environment variables permanently
```

### Build Error: "Project file not found"
**Problem:** Wrong working directory

**Solution:**
```bash
# Always run from project root:
cd C:\Users\allen\Desktop\Alexander\Alexander
python automation_client.py build-editor
```

### Automation Client Not Found
**Problem:** Python can't find automation_client.py

**Solution:**
```bash
# Verify file exists:
ls automation_client.py

# Run with full path if needed:
python C:\Users\allen\Desktop\Alexander\Alexander\automation_client.py build-editor
```

### Profiling Shows No Data
**Problem:** PIE session didn't run long enough

**Solution:**
- Profiling runs for 60 seconds by default
- Check `automation_log.txt` for errors
- Ensure no editor popups blocking PIE start

### Pak Creation Fails
**Problem:** Missing cooked content

**Solution:**
```bash
# Do a full build first:
python automation_client.py build-shipping

# Then create pak:
python automation_client.py create-pak
```

### Verifying UE 5.6 Fixes

#### Check for Remaining TMap Issues
```bash
# Search for UPROPERTY TMap declarations:
grep -r "UPROPERTY.*TMap" Source/
```

Should return no results in .h files if all fixed.

#### Verify Build Success
```bash
python automation_client.py build-editor
```

Check `automation_log.txt` for:
- `BUILD SUCCESSFUL`
- No compilation errors
- Exit code 0

#### Test in Editor
1. Open project in UE editor
2. Open affected classes in Blueprint editor
3. Verify wrapper struct fields appear correctly
4. Check no compilation errors in Output Log

## Quick Diagnostics

### Check Automation Setup
```bash
# Verify Python dependencies:
pip list | grep requests

# Test automation client:
python automation_client.py --help
```

### Check Build Environment
```bash
# Verify UE installation:
ls "$env:UE_ROOT\Engine\Binaries\DotNET\UnrealBuildTool"

# Verify project file:
ls Alexander.uproject
```

### View Recent Logs
```bash
# Last 50 lines of automation log:
tail -50 automation_log.txt

# Search for errors:
grep "ERROR" automation_log.txt
grep "FAILED" automation_log.txt
```

## Common Workflows

### Daily Development Build
```bash
python automation_client.py build-editor
# Open editor and work
# Test changes in PIE
```

### Pre-Commit Check
```bash
python automation_client.py build-editor
# Review automation_log.txt
# If successful, commit changes
```

### Release Preparation
```bash
python automation_client.py nightly-build
# Check all outputs:
# - Build logs
# - Profiling reports
# - Pak files
```

### Fixing UE 5.6 Compilation Errors
1. Note the TMap error from build log
2. Locate the .h file mentioned in error
3. Create wrapper struct for that TMap type
4. Replace TMap with TArray of wrapper struct
5. Update .cpp file to use new array pattern
6. Rebuild: `python automation_client.py build-editor`
7. Repeat until clean build

## File Locations

- **Automation logs:** `automation_log.txt`
- **Session state:** `session_state.json`
- **Task queue:** `execution_queue.json`
- **Screenshots:** `Screenshots/`
- **Build output:** `Binaries/Win64/`
- **Cooked content:** `Saved/Cooked/`
- **Profiling data:** `Saved/Profiling/`

## Quick Tips

- Always run automation from project root directory
- Check `automation_log.txt` after any build command
- Wrapper structs must be declared BEFORE the class that uses them
- Use descriptive names for wrapper structs (FStringFloatEntry, not FEntry)
- Add constructors to wrapper structs for easier initialization
- Keep helper functions for common lookup patterns
- Test in editor after fixing TMap issues to verify Blueprint exposure works
