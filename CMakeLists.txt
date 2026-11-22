cmake_minimum_required(VERSION 3.20)

project(CMinorCompiler VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cminorcc
    src/main.cpp
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/semantic/semantic.cpp
)

target_include_directories(cminorcc PRIVATE include)

if(MSVC)
    target_compile_options(cminorcc PRIVATE /W4)
else()
    target_compile_options(cminorcc PRIVATE -Wall -Wextra -Wpedantic)
endif()
