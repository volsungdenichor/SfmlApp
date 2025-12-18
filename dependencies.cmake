include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(mx
    GIT_REPOSITORY https://github.com/volsungdenichor/mx.git
    GIT_TAG main
    EXCLUDE_FROM_ALL
    SYSTEM)

FetchContent_MakeAvailable(mx SFML)

include_directories(${mx_SOURCE_DIR}/include)
