cmake_minimum_required(VERSION 3.5)

set(project QDiscordLib)
project(
    ${project}
    DESCRIPTION "Yet another discord lib written in Qt"
    LANGUAGES CXX
)

# --- Find and configure Qt ---
find_package(
    Qt5
    COMPONENTS
        Core
	Network
	WebSockets
	Concurrent
    REQUIRED
)
if (NOT Qt5Core_FOUND)
    message(FATAL_ERROR "No Qt installation found, aborting")
endif()
message("Found Qt ${Qt5Core_VERSION}")
# -----------------------------

# --- Set global C++ CMake settings ---
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(VERBOSE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
# -------------------------------

# --- Set compiler flags ---
add_definitions(-DBUILDER_STATIC_DEFINE)
if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
    add_definitions(-DDEBUG)
endif()
if(MSVC)
    # Remove all previously declared flags before adding new ones
    string(REGEX REPLACE "[/-]W[0-4] ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-][ML][DT]d? ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-]O[12bdgistxy] ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-]DEBUG:[a-zA-Z]+ ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

    # Enable max warnings
    string(APPEND CMAKE_CXX_FLAGS " /W4")

    if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	string(APPEND CMAKE_CXX_FLAGS " /MT /DEBUG:NONE /O2")
    else()
	string(APPEND CMAKE_CXX_FLAGS " /MTd /DEBUG:FULL /Od")
    endif()
else()
    # 64-bit, threading, and verbose
    string(APPEND CMAKE_CXX_FLAGS " -m64 -pthread -Wall -Wextra -pedantic")

    if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	string(APPEND CMAKE_CXX_FLAGS " -fstack-protector -O3 -g0 -s")
    else()
	string(APPEND CMAKE_CXX_FLAGS " -O0 -g3")
    endif()
endif()
# --------------------------

# --- Configure building ---
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5WebSockets_INCLUDE_DIRS}
    ${Qt5Concurrent_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/DiscordClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/serialization.h
)

set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DiscordClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serialization.cpp
)

add_executable(${project}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${${project}_HEADERS}
    ${${project}_SOURCES}
)

target_link_libraries(${project}
    ${Qt5Core_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5WebSockets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
)
