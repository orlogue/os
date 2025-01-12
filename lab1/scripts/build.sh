#!/bin/bash

cd "$(dirname "$0")/.."

git pull

mkdir -p build

cd build

cmake ..

cmake --build .

echo "Build completed. Executable file is in build/" 