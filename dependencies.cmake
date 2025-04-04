include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)

FetchContent_Declare(ferrugo-core
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo-core.git
    GIT_TAG main)

FetchContent_Declare(ferrugo-alg
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo-alg.git
    GIT_TAG main)

FetchContent_MakeAvailable(ferrugo-core ferrugo-alg SFML)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE sfml-graphics)
target_compile_features(main PRIVATE cxx_std_17)

include_directories(/usr/include/freetype2)

include_directories(${ferrugo-core_SOURCE_DIR}/include)
include_directories(${ferrugo-alg_SOURCE_DIR}/include)
