cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "-O3")
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(dependencies.cmake)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE sfml-graphics zx::sequence zx::functional)
target_compile_features(main PRIVATE cxx_std_17)
