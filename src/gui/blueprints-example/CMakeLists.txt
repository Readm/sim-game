add_example_executable(blueprints-example
    blueprints-example.cpp
    utilities/builders.h
    utilities/drawing.h
    utilities/widgets.h
    utilities/builders.cpp
    utilities/drawing.cpp
    utilities/widgets.cpp
)

# 添加对cpp-httplib的依赖
target_include_directories(blueprints-example PRIVATE ${CMAKE_SOURCE_DIR}/../../external/cpp-httplib)

# 链接到sim库
target_link_libraries(blueprints-example PRIVATE pthread)

# 引入sim库的类型定义头文件
target_include_directories(blueprints-example PRIVATE ${CMAKE_SOURCE_DIR}/../sim/include)
