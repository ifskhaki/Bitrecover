@echo off
REM ============================================
REM Bitrecover - One-Click Run Script
REM ============================================
echo.
echo ╔═══════════════════════════════════════════════════════════════╗
echo ║          Bitrecover - Multi-GPU Bitcoin Key recover          ║
echo ╚═══════════════════════════════════════════════════════════════╝
echo.

REM Check if build directory exists
if not exist "build\bin\bitrecover.exe" (
    echo ❌ ERROR: bitrecover.exe not found!
    echo.
    echo Please build the project first:
    echo   1. Open Command Prompt or PowerShell
    echo   2. cd to the bitre cover directory
    echo   3. Run: mkdir build
    echo   4. Run: cd build
    echo   5. Run: cmake ..
    echo   6. Run: cmake --build . --config Release
    echo.
    pause
    exit /b 1
)

REM Check if address.txt exists
if not exist "address.txt" (
    echo ⚠️  WARNING: address.txt not found!
    echo Creating sample address.txt...
    echo 1H5XRGRbUtmuatAj1xpAm7zsPuNJSBsC76 > address.txt
    echo 1H5Ydru5W6x2FqJTtqUDkpDy3LaKnHXEjM >> address.txt
    echo ✅ Created sample address.txt
    echo.
)

REM Check if config.json exists
if not exist "config\config.json" (
    echo ❌ ERROR: config\config.json not found!
    echo Please create your config file first.
    echo.
    pause
    exit /b 1
)

echo ✅ All files ready!
echo.
echo 🚀 Starting Bitrecover...
echo.
echo Press Ctrl+C to stop
echo ═══════════════════════════════════════════════════════════════
echo.

REM Run bitrecover
build\bin\bitrecover.exe --config config\config.json

echo.
echo ═══════════════════════════════════════════════════════════════
echo.
pause
