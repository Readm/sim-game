# Include FetchContent module
include(FetchContent)

# Fetch dependencies
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.7
)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_Declare(
    flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v23.5.26
    CMAKE_ARGS
        -DFLATBUFFERS_BUILD_TESTS=OFF
        -DFLATBUFFERS_BUILD_FLATC=ON
        -DFLATBUFFERS_BUILD_SHAREDLIB=OFF
        -DFLATBUFFERS_STATIC_LIB=ON
        -DFLATBUFFERS_BUILD_SCHEMA=OFF
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_Declare(
    ImGuiFileDialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG v0.6.5
)
FetchContent_MakeAvailable(imgui doctest flatbuffers nlohmann_json ImGuiFileDialog)

# Configure FlatBuffers build options
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "Disable FlatBuffers tests" FORCE)
set(FLATBUFFERS_BUILD_FLATC ON CACHE BOOL "Build the flatc compiler" FORCE)
set(FLATBUFFERS_BUILD_SHAREDLIB OFF CACHE BOOL "Build FlatBuffers as a shared library" FORCE)
set(FLATBUFFERS_STATIC_LIB ON CACHE BOOL "Force FlatBuffers to build as a static library" FORCE)
set(FLATBUFFERS_BUILD_SCHEMA OFF CACHE BOOL "Disable schema compilation for samples" FORCE)

# Force FlatBuffers to build as a STATIC library
if(TARGET flatbuffers)
    set_target_properties(flatbuffers PROPERTIES INTERFACE_LINK_LIBRARIES "")
    set_target_properties(flatbuffers PROPERTIES LINKER_LANGUAGE CXX)
endif()

# Define ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${ImGuiFileDialog_SOURCE_DIR}/ImGuiFileDialog.cpp
)

# Ensure ImGuiFileDialog includes imgui and nlohmann_json headers
target_include_directories(ImGuiFileDialog PRIVATE 
    ${imgui_SOURCE_DIR}
    ${nlohmann_json_SOURCE_DIR}/single_include
)

# Find and link GLEW
find_package(GLEW REQUIRED)

# Find and link OpenGL
find_package(OpenGL REQUIRED)

# Include directories
set(INCLUDE_DIRS
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${doctest_SOURCE_DIR}/doctest
    ${doctest_BINARY_DIR}
    ${flatbuffers_SOURCE_DIR}/include
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${nlohmann_json_SOURCE_DIR}/single_include
    ${ImGuiFileDialog_SOURCE_DIR}
)

include_directories(${INCLUDE_DIRS})

# Link libraries
set(LINK_LIBS
    doctest::doctest
    flatbuffers
    glfw
    ${GLEW_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    dl
)
