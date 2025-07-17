@echo off
echo Building Chatty Application...
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed!
    echo Please ensure Qt5 or Qt6 is installed and accessible.
    echo.
    echo For Qt6: Install Qt6 from https://www.qt.io/download-qt-installer
    echo For Qt5: Install Qt5 development libraries
    echo.
    pause
    exit /b 1
)

REM Build the project
echo.
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed!
    echo Check the error messages above for details.
    echo.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable location: bin/Chatty.exe
echo.
pause 