#!/bin/bash

echo "Building Chatty Application..."
echo

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo
    echo "CMake configuration failed!"
    echo "Please ensure Qt5 or Qt6 is installed and accessible."
    echo
    echo "Ubuntu/Debian: sudo apt install qt6-base-dev qt6-tools-dev cmake"
    echo "               or sudo apt install qt5-default qttools5-dev cmake"
    echo "macOS:         brew install qt cmake"
    echo "Fedora:        sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake"
    echo
    exit 1
fi

# Build the project
echo
echo "Building project..."
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo
    echo "Build failed!"
    echo "Check the error messages above for details."
    echo
    exit 1
fi

echo
echo "Build completed successfully!"
echo "Executable location: bin/Chatty"
echo

# Make executable runnable
chmod +x bin/Chatty 2>/dev/null || true

echo "To run the application:"
echo "  ./bin/Chatty"
echo 