@echo off
REM Ship Content Import Script
REM This script documents the manual import process for Unreal Engine

echo ========================================
echo Ship Content Import Instructions
echo ========================================
echo.
echo 1. Open Unreal Editor
echo.
echo 2. Create Ship Parts Data Table:
echo    - Right-click in Content Browser
echo    - Miscellaneous ^> Data Table
echo    - Select row structure: ShipPartData
echo    - Name: DT_ShipParts
echo    - Save to: Content/Data/
echo.
echo 3. Import Ship Parts CSV:
echo    - Open DT_ShipParts
echo    - Click "Import" button
echo    - Select: Content/Data/ShipParts.csv
echo    - Verify all rows imported correctly
echo.
echo 4. Create Ship Skins Data Table:
echo    - Right-click in Content Browser
echo    - Miscellaneous ^> Data Table
echo    - Select row structure: ShipSkinData
echo    - Name: DT_ShipSkins
echo    - Save to: Content/Data/
echo.
echo 5. Import Ship Skins CSV:
echo    - Open DT_ShipSkins
echo    - Click "Import" button
echo    - Select: Content/Data/ShipSkins.csv
echo    - Verify all rows imported correctly
echo.
echo 6. Link Data Tables to ShipCustomizationComponent:
echo    - Open your ship Blueprint
echo    - Find ShipCustomizationComponent
echo    - Set ShipPartsDataTable to DT_ShipParts
echo    - Set ShipSkinsDataTable to DT_ShipSkins
echo.
echo ========================================
echo Import process documented above
echo ========================================
pause
