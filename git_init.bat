@echo off
echo Configuring Git...
git config user.name "Alexander Dev"
git config user.email "dev@alexander.local"

echo Adding all files...
git add .

echo Creating initial commit...
git commit -m "Initial commit - Alexander VR Space Simulation with complete CI/CD pipeline, testing framework (229 tests), and comprehensive documentation"

echo.
echo Git repository initialized successfully!
echo.
echo Next steps:
echo 1. Create a GitHub repository
echo 2. Run: git remote add origin https://github.com/YOUR_USERNAME/Alexander.git
echo 3. Run: git branch -M main
echo 4. Run: git push -u origin main
echo.
pause
