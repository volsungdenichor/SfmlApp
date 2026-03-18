include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

set(ZX_BUILD_SEQUENCE ON CACHE BOOL "" FORCE)
set(ZX_BUILD_FUNCTIONAL ON CACHE BOOL "" FORCE)
set(ZX_BUILD_MAT ON CACHE BOOL "" FORCE)
set(ZX_BUILD_GEOMETRY ON CACHE BOOL "" FORCE)

set(ZX_SOURCE_DIR "" CACHE PATH "Path to local zx source directory")

if(ZX_SOURCE_DIR)
    message(STATUS "Using local zx source from: ${ZX_SOURCE_DIR}")
    FetchContent_Declare(zx
        SOURCE_DIR ${ZX_SOURCE_DIR}
        EXCLUDE_FROM_ALL
        SYSTEM)
else()
    FetchContent_Declare(zx
        GIT_REPOSITORY git@github.com:volsungdenichor/zx.git
        GIT_TAG main
        EXCLUDE_FROM_ALL
        SYSTEM)
endif()

FetchContent_MakeAvailable(zx SFML)
