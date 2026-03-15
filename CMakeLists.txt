cmake_minimum_required(VERSION 3.22)
project(DrivingML LANGUAGES C ASM VERSION 0.1.0)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(ignoreMe "${CMAKE_CXX_COMPILER}")
set(CMAKE_CXX_COMPILER_WORKS 1)

file(GLOB_RECURSE drivingml_SRC include/drivingml/*.h src/drivingml/*.c src/drivingml/*.s)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything -Wno-pre-c2x-compat -Wno-c++98-compat -Wno-declaration-after-statement -Wno-unsafe-buffer-usage -Wno-format-nonliteral -Wno-unused-function -Wno-unused-parameter -Werror=vla")

add_definitions(-D_USE_MATH_DEFINES)

add_subdirectory(lib/Mana buildmana)

set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")

file(GLOB_RECURSE GLSL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/**/*.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/**/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/**/*.comp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/*.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl/*.comp)

foreach(GLSL ${GLSL_SOURCE_FILES})
        # Get the relative path of the GLSL file from the glsl directory
        file(RELATIVE_PATH RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/glsl ${GLSL})

        # Extract the directory path and file name
        get_filename_component(RELATIVE_DIR ${RELATIVE_PATH} DIRECTORY)
        get_filename_component(FILE_NAME ${GLSL} NAME)

        # If the relative directory is not empty, append a slash to it for proper path construction
        if(NOT "${RELATIVE_DIR}" STREQUAL "")
                set(RELATIVE_DIR "${RELATIVE_DIR}/")
        endif()

        # Construct the SPIR-V output path
        set(SPIRV "${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/spirv/${RELATIVE_DIR}${FILE_NAME}.spv")

        # Create the directory for the SPIR-V file if it doesn't exist
        get_filename_component(SPIRV_DIR "${SPIRV}" DIRECTORY)
        file(MAKE_DIRECTORY ${SPIRV_DIR})

        # The custom command to compile the GLSL to SPIR-V
        add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling ${GLSL} to SPIR-V"
        )

        # Append the SPIR-V file to the list of outputs
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

# Add all SPIR-V binaries to a custom target
add_custom_target(
        CompileShaders ALL
        DEPENDS ${SPIRV_BINARY_FILES}
        COMMENT "SPIR-V Shader Compilation Target"
)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

file(GLOB_RECURSE ALL_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/*)

add_custom_target(Assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets ${PROJECT_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/assets)

set(includeList ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/include "$ENV{VULKAN_SDK}/Include")

add_executable(DrivingML ${drivingml_SRC})
target_compile_options(DrivingML PRIVATE -march=native)
target_include_directories(DrivingML PUBLIC ${includeList})
add_dependencies(DrivingML Shaders Assets)
set(DRIVINGML_BUILD_LIBRARIES DrivingML mana)
target_link_libraries(${DRIVINGML_BUILD_LIBRARIES})
set_target_properties(DrivingML PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/run")
