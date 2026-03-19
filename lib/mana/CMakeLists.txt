cmake_minimum_required(VERSION 3.22)
project(mana LANGUAGES C VERSION 0.1.0)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(ignoreMe "${CMAKE_CXX_COMPILER}")
set(CMAKE_CXX_COMPILER_WORKS 1)

file(GLOB_RECURSE mana_SRC include/*.h src/*.c)

add_library(mana STATIC ${mana_SRC})

target_include_directories(mana PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (WIN32)
    target_include_directories(mana PUBLIC "$ENV{VULKAN_SDK}/Include")
    target_link_libraries(mana "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib" d3d12 dxgi d3dcompiler xinput winusb setupapi ws2_32)
elseif(APPLE)
    find_library(METAL_FRAMEWORK Metal)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)
    find_library(APPKIT_FRAMEWORK AppKit)
    target_link_libraries(mana ${METAL_FRAMEWORK} ${FOUNDATION_FRAMEWORK} ${QUARTZCORE_FRAMEWORK} ${APPKIT_FRAMEWORK})
endif()
