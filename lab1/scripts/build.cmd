@echo off

cd /d "%~dp0\.."

git pull

if not exist build mkdir build

cd build

cmake ..

cmake --build .

echo Build completed. Executable file is in build/
pause 