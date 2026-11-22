# C Minor Compiler

## Project Overview
C Minor Compiler is a partial compiler construction project implemented using C++17.

## Technologies
- C++17
- CMake
- GCC
- LLVM 23
- Ubuntu Linux

## Implemented Components
1. Lexical Analysis
2. Syntax Analysis
3. Abstract Syntax Tree (AST)
4. Basic Semantic Analysis
5. Basic Symbol Table

## Implemented Features
- Keyword and identifier recognition
- Integer literal validation
- Arithmetic and logical operators
- Comment handling
- Function and statement parsing
- Expression precedence
- Variable declaration checking
- Duplicate variable detection
- Undeclared variable detection
- Loop context checking

## Build Instructions
    cmake -S . -B build
    cmake --build build

## Usage
    ./build/cminorcc --check tests/positive/basic.cm
    ./build/cminorcc --dump-tokens tests/positive/basic.cm
    ./build/cminorcc --version

## Current Limitations
This is a partial compiler implementation.

Student IR, LLVM IR generation, assembly generation,
object file generation, executable linking and complete
SRS compliance are not yet implemented.
