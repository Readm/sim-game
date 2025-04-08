cmake_minimum_required(VERSION 3.12)
project(FlowSimulator)

# 设置 imgui-node-editor 根目录
set(IMGUI_NODE_EDITOR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-node-editor)

# 添加 dl 库
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_library(DL_LIBRARY dl REQUIRED)

# 设置编译选项
option(IMGUI_NODE_EDITOR_STANDALONE "Build standalone examples" ON)
option(IMGUI_NODE_EDITOR_STATIC "Build static library" ON)

# 设置 CMake 模块路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# 设置外部依赖目录
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)

# 函数：检查并获取依赖
function(get_dependency NAME GIT_REPO GIT_TAG)
    # 检查本地是否存在
    if(EXISTS ${EXTERNAL_DIR}/${NAME})
        message(STATUS "Using local ${NAME} from external directory")
        set(${NAME}_SOURCE_DIR ${EXTERNAL_DIR}/${NAME} PARENT_SCOPE)
    else()
        message(STATUS "Downloading ${NAME} from ${GIT_REPO}")
        # 下载到external目录
        execute_process(
            COMMAND git clone --depth 1 --branch ${GIT_TAG} ${GIT_REPO} ${EXTERNAL_DIR}/${NAME}
            RESULT_VARIABLE GIT_RESULT
        )
        if(GIT_RESULT EQUAL 0)
            set(${NAME}_SOURCE_DIR ${EXTERNAL_DIR}/${NAME} PARENT_SCOPE)
        else()
            message(WARNING "Failed to clone ${NAME}, falling back to FetchContent")
            include(FetchContent)
            FetchContent_Declare(
                ${NAME}
                GIT_REPOSITORY ${GIT_REPO}
                GIT_TAG ${GIT_TAG}
            )
            FetchContent_MakeAvailable(${NAME})
        endif()
    endif()
endfunction()

# 获取依赖
get_dependency(stb https://github.com/nothings/stb.git master)
get_dependency(scope_guard https://github.com/Neargye/scope_guard.git master)
get_dependency(doctest https://github.com/doctest/doctest.git v2.4.11)
get_dependency(json https://github.com/nlohmann/json.git v3.11.3)

# 添加 imgui-node-editor 库
add_subdirectory(${IMGUI_NODE_EDITOR_ROOT_DIR})

# 创建 stb_image 包配置
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# 创建 ScopeGuard 包配置
add_library(ScopeGuard INTERFACE)
target_include_directories(ScopeGuard INTERFACE ${scope_guard_SOURCE_DIR}/include)

# 创建 json 包配置
add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_include_directories(nlohmann_json INTERFACE ${json_SOURCE_DIR}/include)

# 创建 doctest 包配置
add_library(doctest INTERFACE)
add_library(doctest::doctest ALIAS doctest)
target_include_directories(doctest INTERFACE ${doctest_SOURCE_DIR})

# 添加 application 库
add_subdirectory(${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/application)

# 启用测试
enable_testing()

# 添加 unittest
add_subdirectory(src/sim/unittest)

# 定义函数来添加示例
function(add_example_executable name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE imgui-node-editor application ${DL_LIBRARY})
    target_include_directories(${name} PRIVATE 
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/application/include
    )
endfunction()

# 添加 blueprints-example
add_example_executable(blueprints-example
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/blueprints-example.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/builders.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/drawing.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/widgets.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/builders.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/drawing.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/widgets.cpp
)

# 复制数据文件
file(COPY ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/data/ DESTINATION ${CMAKE_BINARY_DIR}/data)
file(COPY ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/data/ DESTINATION ${CMAKE_BINARY_DIR}/data)

# 链接 dl 库到所有目标
if(DL_LIBRARY)
    link_libraries(${DL_LIBRARY})
endif() 