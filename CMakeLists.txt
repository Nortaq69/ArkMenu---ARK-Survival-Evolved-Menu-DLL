cmake_minimum_required(VERSION 3.16)
project(ArkMenu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX- /MP)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/minhook/include
)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.c"
    "external/imgui/*.cpp"
    "external/minhook/src/*.c"
)

# Create DLL
add_library(ArkMenu SHARED ${SOURCES})

# Link libraries
target_link_libraries(ArkMenu
    d3d11
    dxgi
    d3dcompiler
    user32
    kernel32
    shell32
    ole32
    uuid
    advapi32
    psapi
)

# Copy DLL to game directory after build
add_custom_command(TARGET ArkMenu POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:ArkMenu>
    "C:/Program Files/WindowsApps/Microsoft.FlightSimulator_*/ARK/ShooterGame/Binaries/WinGDK/"
    COMMENT "Copying DLL to ARK directory"
) 