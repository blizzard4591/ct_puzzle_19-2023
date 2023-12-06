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

# Add base folder for better inclusion paths
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")

# Include cereal
include_directories("${PROJECT_SOURCE_DIR}/thirdParty/cereal-1.3.2/include")
# Include LZ4 Stream
include_directories("${PROJECT_SOURCE_DIR}/thirdParty/lz4_stream/include")

# Main Sources
file(GLOB PROJECT_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB PROJECT_SOURCES_CPP ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES_CPP})

if (MSVC)
	include_directories("${PROJECT_SOURCE_DIR}/thirdParty/lz4/include")
	target_link_libraries(${CMAKE_PROJECT_NAME} debug "${PROJECT_SOURCE_DIR}/thirdParty/lz4/x64_Debug/liblz4_static.lib" optimized "${PROJECT_SOURCE_DIR}/thirdParty/lz4/x64_Release/liblz4_static.lib")
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LZ4 REQUIRED liblz4)
	target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${LZ4_INCLUDE_DIR})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${LZ4_LIBRARIES})
endif()

set(CMAKE_CXX_STANDARD 17)
