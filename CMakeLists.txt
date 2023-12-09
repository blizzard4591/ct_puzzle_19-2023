cmake_minimum_required(VERSION 3.16)

project(Ct40terGeburtstagPenguin CXX)

set(CXX ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(PROJECT_CMAKE_SEARCH_PATH "C:/Qt/6.6.0/msvc2019_64" CACHE PATH "Additional Qt search path" )

list(APPEND CMAKE_PREFIX_PATH "${PROJECT_CMAKE_SEARCH_PATH}/lib/cmake/")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}")

# Visual Studio compiler with static runtime libraries
if(MSVC AND MT)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
endif()

if (MSVC)
	add_definitions(/std:c++17)
	add_definitions(/DNOMINMAX)
	add_definitions(/MP)
	add_definitions(/DPSAPI_VERSION=1)
else()
	add_definitions(-std=c++17)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

message(STATUS "Using Qt version ${QT_VERSION_MAJOR}.")

if ("${QT_VERSION_MAJOR}" STREQUAL "6")
	qt_standard_project_setup()
endif()

# Add base folder for better inclusion paths
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")

# Main Sources
file(GLOB PROJECT_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB PROJECT_SOURCES_CPP ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES_CPP})

set(CMAKE_CXX_STANDARD 17)

target_link_libraries(${CMAKE_PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui)
