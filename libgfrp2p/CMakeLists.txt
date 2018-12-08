cmake_minimum_required(VERSION 2.8)
project(GFRR_P2P)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

# Check C++11 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    # add_definitions(-DCOMPILEDWITHC11)
    message(STATUS "Using flag -std=c++11.")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Check libboost-system and libboost-log
find_package(Boost COMPONENTS system log REQUIRED)

include_directories(
${Boost_SYSTEM_INCLUDE_DIRS}
${Boost_LOG_INCLUDE_DIRS}
${PROJECT_SOURCE_DIR}/include
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(${PROJECT_NAME} SHARED
src/discovery.cpp
src/node_table.cpp
src/node.cpp
src/transport.cpp
)

target_link_libraries(${PROJECT_NAME}
${Boost_SYSTEM_LIBRARY}
${Boost_LOG_LIBRARY}
)

# Make executable

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# add_executable(test
# src/app.cpp
# )
