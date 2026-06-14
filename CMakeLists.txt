cmake_minimum_required(VERSION 3.28)
project(Forge11 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
endif()

add_subdirectory(engine)