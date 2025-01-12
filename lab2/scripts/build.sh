#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

echo "Pulling latest changes..."
git pull

mkdir -p build
cd build

echo "Configuring project..."
cmake ..

echo "Building project..."
cmake --build .

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "You can find:"
    echo "- Library in: $(pwd)/lib/process_lib"
    echo "- Test utility in: $(pwd)"
    
    chmod +x process_test
    
    echo -e "\nDo you want to run tests? [y/N]"
    read -r response
    if [[ "$response" =~ ^([yY][eE][sS]|[yY])+$ ]]; then
        ./process_test
    fi
else
    echo "Build failed!"
    exit 1
fi 