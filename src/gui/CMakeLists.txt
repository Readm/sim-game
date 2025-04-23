cmake_minimum_required(VERSION 3.12)

project(imgui-node-editor)

# Define IMGUI_NODE_EDITOR_ROOT_DIR pointing to project root directory
get_filename_component(IMGUI_NODE_EDITOR_ROOT_DIR ${CMAKE_SOURCE_DIR}/.. ABSOLUTE CACHE)

# 设置external库的根目录
set(EXTERNAL_DIR ${IMGUI_NODE_EDITOR_ROOT_DIR}/../external/imgui-node-editor/external)

# 添加crude_json库
set(CRUDE_JSON_DIR ${IMGUI_NODE_EDITOR_ROOT_DIR}/../external/imgui-node-editor)
add_library(crude_json STATIC
    ${CRUDE_JSON_DIR}/crude_json.cpp
)
target_include_directories(crude_json PUBLIC ${CRUDE_JSON_DIR})

# 添加imgui作为项目的一部分
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

# 添加imgui_node_editor作为项目的一部分
set(NODE_EDITOR_DIR ${IMGUI_NODE_EDITOR_ROOT_DIR}/../external/imgui-node-editor)
add_library(imgui_node_editor STATIC
    ${NODE_EDITOR_DIR}/imgui_node_editor.cpp
    ${NODE_EDITOR_DIR}/imgui_node_editor_api.cpp
    ${NODE_EDITOR_DIR}/imgui_canvas.cpp
)
target_include_directories(imgui_node_editor PUBLIC ${NODE_EDITOR_DIR})
target_link_libraries(imgui_node_editor PUBLIC imgui crude_json)

# 添加stb_image支持
set(STB_IMAGE_DIR ${EXTERNAL_DIR}/stb_image)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${STB_IMAGE_DIR})

# 添加ScopeGuard支持
set(SCOPEGUARD_DIR ${EXTERNAL_DIR}/ScopeGuard)
add_library(ScopeGuard INTERFACE)
target_include_directories(ScopeGuard INTERFACE ${SCOPEGUARD_DIR})

# 添加DXSDK支持（仅Windows）
if (WIN32)
    set(DXSDK_DIR ${EXTERNAL_DIR}/DXSDK)
    add_library(dxerr STATIC ${DXSDK_DIR}/src/dxerr.cpp)
    target_include_directories(dxerr PUBLIC "${DXSDK_DIR}/include")
    set_property(TARGET dxerr PROPERTY FOLDER "external")

    set(_DXSDK_Arch x86)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(_DXSDK_Arch x64)
    endif()

    add_library(d3dx11 UNKNOWN IMPORTED)
    set_target_properties(d3dx11 PROPERTIES
        IMPORTED_LOCATION               "${DXSDK_DIR}/lib/${_DXSDK_Arch}/d3dx11.lib"
        IMPORTED_LOCATION_DEBUG         "${DXSDK_DIR}/lib/${_DXSDK_Arch}/d3dx11d.lib"
        INTERFACE_INCLUDE_DIRECTORIES   "${DXSDK_DIR}/include"
        INTERFACE_LINK_LIBRARIES        "$<$<CONFIG:Debug>:dxerr>"
    )
endif()

# Enable solution folders in Visual Studio and Folders in Xcode
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Point CMake where to look for module files.
list(APPEND CMAKE_MODULE_PATH ${IMGUI_NODE_EDITOR_ROOT_DIR}/misc/cmake-modules)

# Node editor use C++14
set(CMAKE_CXX_STANDARD            14)
set(CMAKE_CXX_STANDARD_REQUIRED   YES)

# 设置调试编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -ggdb3)
    add_compile_definitions(DEBUG)
endif()

# Macro that will configure an example application
macro(add_example_executable name)
    project(${name})

    set(_Example_Sources
        ${ARGN}
    )

    #source_group("" FILES ${_Example_Sources})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_Example_Sources})

    file(GLOB _Example_CommonResources CONFIGURE_DEPENDS "${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/data/*")
    file(GLOB _Example_Resources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/data/*")
    #message(FATAL_ERROR "_Example_Resources = ${_Example_Resources}")

    set(_Example_Type)
    if (WIN32)
        set(_Example_Type WIN32)

        set(ApplicationIcon ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/Application/Support/Icon.ico)
        file(TO_NATIVE_PATH "${ApplicationIcon}" ApplicationIcon)
        string(REPLACE "\\" "\\\\" ApplicationIcon "${ApplicationIcon}")
        configure_file(
            ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/Application/Support/Resource.rc.in
            ${CMAKE_CURRENT_BINARY_DIR}/Resource.rc
        )
        source_group(TREE "${IMGUI_NODE_EDITOR_ROOT_DIR}/examples" FILES ${_Example_CommonResources})
        source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_Example_Resources})
        list(APPEND _Example_Resources
            ${CMAKE_CURRENT_BINARY_DIR}/Resource.rc
            ${_Example_CommonResources}
        )
        source_group("resources" FILES ${CMAKE_CURRENT_BINARY_DIR}/Resource.rc)
    elseif (APPLE)
        set(_Example_Type MACOSX_BUNDLE)

        set_source_files_properties(${_Example_Resources} ${_Example_CommonResources} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources/data"
        )
        set(_Example_Icon "${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/application/support/Icon.icns")
        list(APPEND _Example_Resources ${_Example_Icon})
        set_source_files_properties(${_Example_Icon} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
    endif()

    add_executable(${name} ${_Example_Type} ${_Example_Sources} ${_Example_Resources} ${_Example_CommonResources})

    target_link_libraries(${name} PRIVATE imgui imgui_node_editor application)
    
    # 添加 sim/include 路径
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/../sim/include)

    set(_ExampleBinDir ${CMAKE_BINARY_DIR}/bin)

    set_target_properties(${name} PROPERTIES
        FOLDER "examples"
        RUNTIME_OUTPUT_DIRECTORY                "${_ExampleBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${_ExampleBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_ExampleBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${_ExampleBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${_ExampleBinDir}"
        DEBUG_POSTFIX                           _d
        RELWITHDEBINGO_POSTFIX                  _rd
        MINSIZEREL_POSTFIX                      _r
        VS_DEBUGGER_WORKING_DIRECTORY           ${_ExampleBinDir}
        MACOSX_BUNDLE_INFO_PLIST                "${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/application/support/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME               "${PACKAGE_NAME}"
        MACOSX_BUNDLE_GUI_IDENTIFIER            "com.sandbox.collisions"
        MACOSX_BUNDLE_LONG_VERSION_STRING       "${PACKAGE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING      "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}"
        MACOSX_BUNDLE_ICON_FILE                 Icon.icns
    )

    add_custom_command(
        TARGET ${name}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ARGS ${_ExampleBinDir}/data
    )

    set(_ResourceRoot ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(_Resource ROOT "${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/data" ${_Example_CommonResources} ROOT "${CMAKE_CURRENT_SOURCE_DIR}/data" ${_Example_Resources})
        if (_Resource STREQUAL ROOT)
            set(_ResourceRoot FALSE)
            continue()
        elseif(NOT _ResourceRoot)
            set(_ResourceRoot ${_Resource})
            continue()
        endif()

        if ("${_Resource}" MATCHES "\.DS_Store$")
            list(REMOVE_ITEM _Example_Resources ${_Resource})
            list(REMOVE_ITEM _Example_CommonResources ${_Resource})
            continue()
        endif()

        file(RELATIVE_PATH _RelResource ${_ResourceRoot} ${_Resource})

        add_custom_command(
            TARGET ${name}
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ARGS ${_Resource} ${_ExampleBinDir}/data/${_RelResource}
        )
    endforeach()

endmacro()

add_subdirectory(application)

# 移除示例目录的引用
# add_subdirectory(canvas-example)
# add_subdirectory(simple-example)
# add_subdirectory(widgets-example)
# add_subdirectory(basic-interaction-example)
# add_subdirectory(blueprints-example)
