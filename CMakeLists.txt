cmake_minimum_required(VERSION 3.16)
project(dfa_minimizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(minimize
        src/main.cpp
        src/DFA.cpp
        src/DFAReader.cpp
        src/Minimizer.cpp
        src/DotExporter.cpp
        src/TablePrinter.cpp)

target_include_directories(minimize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_options(minimize PRIVATE /W4)
else ()
    target_compile_options(minimize PRIVATE -Wall -Wextra -Wpedantic)
endif ()

# Solo por comodidad: si existe la carpeta examples, su entrada se copia junto
# al ejecutable para poder correrlo desde un IDE sin cambiar el directorio de
# trabajo. El programa en si no depende de este archivo de ninguna manera.
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/input.txt)
    add_custom_command(TARGET minimize POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/input.txt
            $<TARGET_FILE_DIR:minimize>/input.txt)
endif ()
