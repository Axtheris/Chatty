cmake_minimum_required(VERSION 3.16)
project(Chatty VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt (try Qt6 first, then Qt5)
find_package(Qt6 QUIET COMPONENTS Core Widgets Network)
if (Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Found Qt6")
else()
    find_package(Qt5 QUIET COMPONENTS Core Widgets Network)
    if (Qt5_FOUND)
        set(QT_VERSION_MAJOR 5)
        message(STATUS "Found Qt5")
    else()
        message(FATAL_ERROR "Qt5 or Qt6 is required but not found. Please install Qt development libraries.")
    endif()
endif()

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ChatWidget.cpp
    src/MessageWidget.cpp
    src/WelcomeWidget.cpp
    src/OpenRouterAPI.cpp
    src/FileManager.cpp
    src/MarkdownRenderer.cpp
    src/Settings.cpp
    src/SettingsDialog.cpp
)

set(PROJECT_HEADERS
    src/MainWindow.h
    src/ChatWidget.h
    src/MessageWidget.h
    src/WelcomeWidget.h
    src/OpenRouterAPI.h
    src/FileManager.h
    src/MarkdownRenderer.h
    src/Settings.h
    src/SettingsDialog.h
    src/Message.h
)

set(PROJECT_UI
    src/MainWindow.ui
    src/SettingsDialog.ui
)

set(PROJECT_RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME} 
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
    ${PROJECT_RESOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
    )
else()
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
    )
endif()

# Copy resources
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Compiler flags
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif() 