include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)

FetchContent_Declare(ferrugo-core
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo-core.git
    GIT_TAG main)

FetchContent_Declare(ferrugo-alg
    GIT_REPOSITORY https://github.com/volsungdenichor/ferrugo-alg.git
    GIT_TAG main)

FetchContent_MakeAvailable(ferrugo-core ferrugo-alg SFML)

include_directories(${ferrugo-core_SOURCE_DIR}/include)
include_directories(${ferrugo-alg_SOURCE_DIR}/include)
