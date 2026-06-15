# forge11_core: static library, the engine itself (windowing, app, render, ui)
add_library(forge11_core STATIC
    core/Application.cpp
    platform/win32/Win32Window.cpp
    render/Renderer.cpp
    ui/dsl/F11Parser.cpp
    ui/WidgetFactory.cpp
    ui/layout/LayoutEngine.cpp
)

target_include_directories(forge11_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(forge11_core
    PUBLIC
        UNICODE
        _UNICODE
)

if(WIN32)
    target_link_libraries(forge11_core PUBLIC user32 gdi32 d3d12 dxgi d3dcompiler)
endif()

# forge11_abi: shared library exposing the stable C ABI for Forge Studio (P/Invoke)
add_library(forge11_abi SHARED
    abi/forge11_abi.cpp
)

target_link_libraries(forge11_abi PRIVATE forge11_core)

target_compile_definitions(forge11_abi
    PRIVATE
        FORGE11_BUILD_DLL
)

if(WIN32)
    add_executable(hello_window ${CMAKE_SOURCE_DIR}/samples/hello_window/main.cpp)
    target_link_libraries(hello_window PRIVATE forge11_core)
endif()