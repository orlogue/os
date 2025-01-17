#!/bin/bash
set -e  # Exit on any error

echo "Building frontend..."
cd frontend
npm install
npm run build
cd ..

echo "Building backend..."
rm -rf build
mkdir -p build
cd build
cmake ..
make -j$(nproc)

echo "Build completed successfully!"
echo "To run the temperature monitor:"
echo "1. Start socat: socat -d -d pty,raw,echo=0 pty,raw,echo=0"
echo "2. Run temp_sensor with the first port from socat"
echo "3. Run temperature_monitor with the second port from socat"
echo "4. Open http://localhost:8080 in your browser" 