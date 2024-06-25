cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)

FetchContent_Declare(ferrugo
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo.git
    GIT_TAG main)

FetchContent_MakeAvailable(SFML ferrugo)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE sfml-graphics)
target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PRIVATE ${ferrugo_SOURCE_DIR}/include)

include_directories (/usr/include/freetype2)
