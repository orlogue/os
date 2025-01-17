@echo off
echo Building frontend...
cd frontend
call npm install
if %errorlevel% neq 0 exit /b %errorlevel%
call npm run build
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..

echo Building backend...
if exist build rmdir /s /q build
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
if %errorlevel% neq 0 exit /b %errorlevel%
mingw32-make -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Build completed successfully!
echo To run the temperature monitor:
echo 1. Install com0com and create a virtual COM port pair (e.g. COM1 - COM2)
echo 2. Run temp_sensor.exe COM1
echo 3. Run temperature_monitor.exe COM2
echo 4. Open http://localhost:8080 in your browser 