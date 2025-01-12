@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
cd "%PROJECT_DIR%"

echo Pulling latest changes...
git pull

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo CMake not found! Please install CMake and add it to PATH
    exit /b 1
)

if not exist build mkdir build
cd build

echo Configuring project...
cmake ..

echo Building project...
cmake --build .

if %ERRORLEVEL% equ 0 (
    echo Build successful!
    echo You can find:
    echo - Library in: %CD%\lib\process_lib
    echo - Test utility in: %CD%
    
    echo.
    set /p RUN_TESTS="Do you want to run tests? [y/N] "
    if /i "!RUN_TESTS!"=="y" (
        %CD%\process_test.exe
    )
) else (
    echo Build failed!
    exit /b 1
)

endlocal 