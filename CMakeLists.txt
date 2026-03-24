cmake_minimum_required(VERSION 3.22)
project(DrivingML LANGUAGES C ASM VERSION 0.1.0)

# =========================
# Language / Compiler Setup
# =========================
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(ignoreMe "${CMAKE_CXX_COMPILER}")
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything -Wno-pre-c11-compat -Wno-pre-c23-compat -Wno-declaration-after-statement -Wno-unsafe-buffer-usage -Wno-format-nonliteral -Wno-unused-function -Wno-unused-parameter -Werror=vla")

# =========================
# Platform Detection
# =========================
if (APPLE)
    set(MANA_PLATFORM_MACOS TRUE)
elseif (WIN32)
    set(MANA_PLATFORM_WINDOWS TRUE)
endif()

# =========================
# Sources
# =========================
file(GLOB_RECURSE drivingml_SRC include/drivingml/*.h src/drivingml/*.c)

# =========================
# Mana Library
# =========================
add_subdirectory(lib/Mana buildmana)

# =========================
# Shader Compilation
# =========================
set(SHADER_OUTPUTS)

if (MANA_PLATFORM_WINDOWS)
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")

    file(GLOB_RECURSE GLSL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/*.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/*.comp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/**/*.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/**/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/**/*.comp
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})

        file(RELATIVE_PATH REL_PATH
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl
            ${GLSL})

        get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
        get_filename_component(FILE_NAME ${GLSL} NAME)

        if(NOT "${REL_DIR}" STREQUAL "")
            set(REL_DIR "${REL_DIR}/")
        endif()

        set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${REL_DIR}${FILE_NAME}.spv")

        get_filename_component(SPIRV_DIR "${SPIRV}" DIRECTORY)
        file(MAKE_DIRECTORY ${SPIRV_DIR})

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
            COMMENT "Compiling ${GLSL} → SPIR-V"
        )

        list(APPEND SHADER_OUTPUTS ${SPIRV})
    endforeach()
endif()
if (MANA_PLATFORM_MACOS)
    file(GLOB_RECURSE METAL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/metal/*.metal
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/metal/**/*.metal
    )

    foreach(METAL ${METAL_SOURCE_FILES})

        file(RELATIVE_PATH REL_PATH
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/metal
            ${METAL})

        get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
        get_filename_component(FILE_WE ${METAL} NAME_WE)

        if(NOT "${REL_DIR}" STREQUAL "")
            set(REL_DIR "${REL_DIR}/")
        endif()

        set(AIR "${CMAKE_BINARY_DIR}/shaders/${REL_DIR}${FILE_WE}.air")
        set(METALLIB "${CMAKE_BINARY_DIR}/shaders/${REL_DIR}${FILE_WE}.metallib")

        get_filename_component(OUT_DIR "${METALLIB}" DIRECTORY)
        file(MAKE_DIRECTORY ${OUT_DIR})

        add_custom_command(
            OUTPUT ${METALLIB}
            COMMAND xcrun -sdk macosx metal -c ${METAL} -o ${AIR}
            COMMAND xcrun -sdk macosx metallib ${AIR} -o ${METALLIB}
            DEPENDS ${METAL}
            COMMENT "Compiling ${METAL} → metallib"
        )

        list(APPEND SHADER_OUTPUTS ${METALLIB})
    endforeach()
endif()

# -------- Shader Target --------
add_custom_target(
    Shaders ALL
    DEPENDS ${SHADER_OUTPUTS}
)

# =========================
# Assets Copy
# =========================
add_custom_target(Assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets
        ${PROJECT_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${PROJECT_BINARY_DIR}/assets
)

# =========================
# Includes
# =========================
set(includeList
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/include
)

if (MANA_PLATFORM_WINDOWS)
    list(APPEND includeList "$ENV{VULKAN_SDK}/Include")
endif()

# =========================
# Executable
# =========================
add_executable(DrivingML ${drivingml_SRC})

target_compile_options(DrivingML PRIVATE -march=native)

target_include_directories(DrivingML PUBLIC ${includeList})

target_compile_definitions(DrivingML PUBLIC
    $<$<BOOL:${MANA_PLATFORM_MACOS}>:MANA_PLATFORM_MACOS>
    $<$<BOOL:${MANA_PLATFORM_WINDOWS}>:MANA_PLATFORM_WINDOWS>
)

add_dependencies(DrivingML Shaders Assets)

target_link_libraries(DrivingML mana)

set_target_properties(DrivingML PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/run"
)
