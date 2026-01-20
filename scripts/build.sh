#!/bin/bash
set -e

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Navigate to build directory and run CMake + make
cd build
cmake ..
make

echo "Build completed successfully!"