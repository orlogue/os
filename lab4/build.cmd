@echo off

:: Create build directory if it doesn't exist
if not exist build mkdir build

:: Navigate to build directory
cd build

:: Generate build files with CMake
cmake ..

:: Build the project
cmake --build . --config Release

echo Build complete. Executables are in the build directory.
pause 