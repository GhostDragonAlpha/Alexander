@echo off
REM Ship Customization Content Pipeline - Quick Demo
REM This script demonstrates the complete workflow

echo ================================================================================
echo              Ship Customization Content Pipeline - Quick Demo
echo ================================================================================
echo.
echo This demo will show you the complete content creation workflow:
echo   1. Create sample content using the interactive tool
echo   2. Validate the content
echo   3. Export to Unreal format
echo   4. Show you the generated files
echo.
echo Press any key to start the demo...
pause >nul

echo.
echo ================================================================================
echo STEP 1: Interactive Content Creator
echo ================================================================================
echo.
echo The ship_content_creator.py tool provides a menu-driven interface.
echo.
echo Features:
echo   - Create ship parts with guided prompts
echo   - Auto-balanced stat generation
echo   - Quick batch generator for complete sets
echo   - Built-in validation
echo   - Export to Unreal format
echo.
echo Press any key to see the tool...
pause >nul

echo.
echo Starting content creator...
echo (Exit the tool by pressing 0 when ready)
echo.
python ship_content_creator.py

echo.
echo ================================================================================
echo STEP 2: Review Generated Content
echo ================================================================================
echo.
echo Content has been exported to: ./content_output/
echo.
echo Let's view the generated files...
echo.
pause

if exist content_output\ship_parts.json (
    echo.
    echo === Ship Parts (ship_parts.json) ===
    echo.
    type content_output\ship_parts.json
    echo.
    echo.
)

if exist content_output\ship_skins.json (
    echo.
    echo === Ship Skins (ship_skins.json) ===
    echo.
    type content_output\ship_skins.json
    echo.
    echo.
)

echo.
echo ================================================================================
echo STEP 3: Batch Import to Unreal
echo ================================================================================
echo.
echo The batch_import_content.py script will:
echo   - Auto-detect file types (parts, skins, configs)
echo   - Create Data Tables if missing
echo   - Import all content to Unreal Engine
echo   - Validate structure
echo   - Report success/failures
echo.
echo Usage:
echo   python batch_import_content.py ./content_output
echo.
echo (Skipping actual import in this demo - requires Unreal Editor)
echo.
pause

echo.
echo ================================================================================
echo STEP 4: Content Organization
echo ================================================================================
echo.
echo The organize_content.py script will:
echo   - Create proper folder structure in Unreal
echo   - Move assets to correct locations
echo   - Validate naming conventions
echo   - Generate documentation
echo.
echo Folder structure created:
echo.
echo /Game/ShipCustomization/
echo   +-- DataTables/
echo   +-- DataAssets/
echo   +-- Materials/
echo   +-- Meshes/
echo   +-- Icons/
echo   +-- Textures/
echo   +-- Blueprints/
echo.
echo Usage:
echo   python organize_content.py
echo.
pause

echo.
echo ================================================================================
echo STEP 5: Complete Workflow Summary
echo ================================================================================
echo.
echo Complete workflow from concept to in-game:
echo.
echo  1. DESIGN
echo     - Concept: Stats, visuals, gameplay role
echo.
echo  2. CREATE CONTENT
echo     - Use: python ship_content_creator.py
echo     - Interactive prompts with validation
echo     - Auto-balanced stats
echo.
echo  3. VALIDATE
echo     - Built-in validation in content creator
echo     - Check for errors before export
echo.
echo  4. EXPORT
echo     - Export to JSON (Unreal-ready format)
echo     - Files saved to ./content_output/
echo.
echo  5. IMPORT TO UNREAL
echo     - Use: python batch_import_content.py ./content_output
echo     - Auto-creates Data Tables
echo     - Imports all content
echo.
echo  6. ORGANIZE
echo     - Use: python organize_content.py
echo     - Creates folder structure
echo     - Organizes assets
echo.
echo  7. TEST IN-GAME
echo     - Playtest in PIE (Play In Editor)
echo     - Verify stats and balance
echo.
echo  8. ITERATE
echo     - Adjust stats based on testing
echo     - Re-export and re-import
echo     - Repeat until perfect!
echo.
echo.
echo ================================================================================
echo Documentation Available
echo ================================================================================
echo.
echo Complete guides available:
echo.
echo   CONTENT_CREATION_WORKFLOW.md    - Complete workflow guide
echo   DESIGNER_ONBOARDING.md           - Day-by-day onboarding
echo   CONTENT_PIPELINE_SUMMARY.md      - Technical overview
echo   ASSET_NAMING_CONVENTIONS.md      - Naming rules (auto-gen)
echo.
echo Example content:
echo   examples/example_ship_parts.json
echo   examples/example_ship_skins.json
echo   examples/example_physics_config_balanced.json
echo.
echo.
echo ================================================================================
echo Quick Reference
echo ================================================================================
echo.
echo Common commands:
echo.
echo   Create content:
echo     python ship_content_creator.py
echo.
echo   Import to Unreal:
echo     python batch_import_content.py ./content_output
echo.
echo   Organize assets:
echo     python organize_content.py
echo.
echo   Validate naming:
echo     python organize_content.py --validate --auto-fix
echo.
echo   Generate docs:
echo     python organize_content.py --generate-doc
echo.
echo.
echo ================================================================================
echo Demo Complete!
echo ================================================================================
echo.
echo Next steps:
echo   1. Review the documentation in CONTENT_CREATION_WORKFLOW.md
echo   2. Try creating your own content with ship_content_creator.py
echo   3. Follow the onboarding guide in DESIGNER_ONBOARDING.md
echo.
echo Press any key to exit...
pause >nul
