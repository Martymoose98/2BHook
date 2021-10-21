# CMake complains if you don't give it a minimum version
cmake_minimum_required(VERSION 3.4)
project(2BHook CXX ASM_MASM)

# Tell MSVC to enable modern C++ features
set(CMAKE_CXX_STANDARD 20)

# Specify imgui's sources
set(IMGUI_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_SOURCES
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui.cpp
    
    backends/imgui_impl_dx11.cpp
    backends/imgui_impl_win32.cpp
)

# Add directory path to imgui sources
list(TRANSFORM IMGUI_SOURCES PREPEND "${IMGUI_BASE_DIR}/")

# Define our sources
set(SOURCES
    DLLMain.cpp

    Hooks.cpp
    VirtualTableHook.cpp
    MemoryDeviceHook.cpp

    Configuration.cpp
    Console.cpp
    Globals.cpp
    Log.cpp
    mot.cpp
    Overlay.cpp
    Renderer.cpp

    Vector2.cpp
    Vector3.cpp
    Matrix4x4.cpp

    CreateEntityHook.asm
    fastsqrt.asm
    LoadWordBlacklistHook.asm
    SaveFileIOHook.asm
    sincos.asm

    ${IMGUI_SOURCES}
)

# Build a DLL containing our sources
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Add includes
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${IMGUI_BASE_DIR})
