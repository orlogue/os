#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Generate build files with CMake
cmake ..

# Build the project
cmake --build .

echo "Build complete. Executables are in the build directory." 