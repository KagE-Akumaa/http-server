cmake_minimum_required(VERSION 3.20)

project(http-server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(http-server
    src/main.cpp
    src/router.cpp
    src/ClientSocket.cpp
    src/FileReader.cpp
    src/Http_Parser.cpp
    src/Http_Response.cpp
    src/Http_Server.cpp
    src/MimeResolver.cpp
    src/StaticFileHandler.cpp
)

target_include_directories(http-server PRIVATE include)

target_compile_options(http-server PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
