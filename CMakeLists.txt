cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)

FetchContent_Declare(ferrugo-core
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo-core.git
    GIT_TAG main)

FetchContent_Declare(ferrugo-seq
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo-seq.git
    GIT_TAG main)

FetchContent_Declare(ferrugo-geo
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo-geo.git
    GIT_TAG main)

FetchContent_MakeAvailable(ferrugo-core ferrugo-seq ferrugo-geo SFML)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE sfml-graphics)
target_compile_features(main PRIVATE cxx_std_17)

include_directories(/usr/include/freetype2)

include_directories(${ferrugo-core_SOURCE_DIR}/include)
include_directories(${ferrugo-seq_SOURCE_DIR}/include)
include_directories(${ferrugo-geo_SOURCE_DIR}/include)
