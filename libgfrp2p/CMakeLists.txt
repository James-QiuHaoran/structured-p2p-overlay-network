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
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
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

# Check Boost
add_definitions(-D BOOST_ALL_DYN_LINK)
find_package(Boost 1.58.0 COMPONENTS log random REQUIRED)

# Check Protobuf
find_package(Protobuf 3.0.0 REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
    ${PROJECT_SOURCE_DIR}/evaluation/proto/bootstrap_message.proto
)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

# For DLL platforms the DLL part of a shared library is treated as a runtime target 
# and the corresponding import library is treated as an archive target.
# For non-DLL platforms shared libraries are treated as library targets. 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Make library

add_library(${PROJECT_NAME} STATIC
    src/discovery.cpp
    src/node_table.cpp
    src/node.cpp
    src/transport.cpp
    src/peer_manager.cpp
    src/message.cpp
    src/node_table_eth.cpp
    src/peer_manager_eth.cpp
)

target_link_libraries(${PROJECT_NAME}
    Boost::log
    Boost::random
)

# Make executables

add_executable(app
    src/app.cpp
)

add_executable(app_eth
    src/app_eth.cpp
)

target_link_libraries(app
    ${PROJECT_NAME}
)

target_link_libraries(app_eth
    ${PROJECT_NAME}
)

# add_executable(bootstrap_message
#     test/bootstrap_message.cpp
#     ${PROTO_HDRS}
#     ${PROTO_SRCS}
# )

# target_include_directories(bootstrap_message
#     PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
# )

# target_link_libraries(bootstrap_message
#     protobuf::libprotobuf
# )
