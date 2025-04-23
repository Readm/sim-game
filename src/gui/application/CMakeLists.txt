project(application)

set(_Application_Sources
    include/application.h
    source/application.cpp
    source/entry_point.cpp
    source/imgui_extra_keys.h
    source/config.h.in
    source/setup.h
    source/platform.h
    source/platform_win32.cpp
    source/platform_glfw.cpp
    source/renderer.h
    source/renderer_dx11.cpp
    source/renderer_ogl3.cpp
)

add_library(application STATIC)

target_include_directories(application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(application PUBLIC imgui)
target_link_libraries(application PRIVATE stb_image ScopeGuard)

if (WIN32)
    list(APPEND _Application_Sources
        source/imgui_impl_dx11.cpp
        source/imgui_impl_dx11.h
        source/imgui_impl_win32.cpp
        source/imgui_impl_win32.h
    )

    target_link_libraries(application PRIVATE d3d11.lib d3dcompiler.lib d3dx11)
else()
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3 REQUIRED)

    if (APPLE)
        target_link_libraries(application PRIVATE
            "-framework CoreFoundation"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
        )
    endif()
endif()

if (OpenGL_FOUND)
    set(HAVE_OPENGL YES)

    target_include_directories(application PRIVATE ${OPENGL_INCLUDE_DIR})
    target_link_libraries(application PRIVATE ${OPENGL_gl_LIBRARY})
    list(APPEND _Application_Sources
        source/imgui_impl_opengl3.cpp
        source/imgui_impl_opengl3.h
        source/imgui_impl_opengl3_loader.h
    )
endif()

if (glfw3_FOUND)
    set(HAVE_GLFW3 YES)

    list(APPEND _Application_Sources
        source/imgui_impl_glfw.cpp
        source/imgui_impl_glfw.h
    )
    target_link_libraries(application PRIVATE
        glfw
    )
endif()

configure_file(
    source/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/source/config.h
)

target_compile_definitions(application PRIVATE
    #BACKEND_CONFIG=IMGUI_GLFW
    #RENDERER_CONFIG=IMGUI_OGL3
)

target_include_directories(application PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/source)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_Application_Sources})

target_sources(application PRIVATE ${_Application_Sources})

set_property(TARGET application PROPERTY FOLDER "examples")