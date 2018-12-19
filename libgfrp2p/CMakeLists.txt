cmake_minimum_required(VERSION 3.1)
project(GFRRP2P)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

# Check C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Cross-platform compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
    # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} some other flags")
    # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

endif()

if(WIN32)
    # We have to set _WIN32_WINNT for Asio
    if(${CMAKE_SYSTEM_VERSION} EQUAL 10) # Windows 10
        add_definitions(-D _WIN32_WINNT=0x0A00)
    elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.3) # Windows 8.1
        add_definitions(-D _WIN32_WINNT=0x0603)
    elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.2) # Windows 8
        add_definitions(-D _WIN32_WINNT=0x0602)
    elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.1) # Windows 7
        add_definitions(-D _WIN32_WINNT=0x0601)
    elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.0) # Windows Vista
        add_definitions(-D _WIN32_WINNT=0x0600)
    else() # Windows XP (5.1)
        add_definitions(-D _WIN32_WINNT=0x0501)
    endif()
endif()

# For DLL platforms the DLL part of a shared library is treated as a runtime target 
# and the corresponding import library is treated as an archive target.
# For non-DLL platforms shared libraries are treated as library targets. 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Check Boost
# FindBoost doesn't seem to work on Windows: work around

if(WIN32) 
    find_package(Boost 1.58.0 REQUIRED)
else()
    find_package(Boost 1.58.0 COMPONENTS system log REQUIRED)
endif()

include_directories(
${Boost_INCLUDE_DIRS}
${PROJECT_SOURCE_DIR}/include
)

if (WIN32)
    set(LINK_TYPE STATIC)
else()
    set(LINK_TYPE SHARED)
endif()

add_library(${PROJECT_NAME} ${LINK_TYPE}
src/discovery.cpp
src/node_table.cpp
src/node.cpp
src/transport.cpp
src/peer_manager.cpp
)

target_link_libraries(${PROJECT_NAME}
${Boost_SYSTEM_LIBRARY}
${Boost_LOG_LIBRARY}
)

# Make executables

# add_executable(test_udp
# test/udp.cpp
# )

# FindBoost doesn't seem to work on Windows: work around
# if(WIN32) 
link_directories(
${Boost_LIBRARY_DIRS}
)

# else()
target_link_libraries(# test_udp
${PROJECT_NAME}
${Boost_SYSTEM_LIBRARY}
${Boost_LOG_LIBRARY}
)
# endif()
