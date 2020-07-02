cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
## Use -std=c++11 as default.
set(CMAKE_CXX_STANDARD 11)
## Disable C++ extensions
set(CMAKE_CXX_EXTENSIONS OFF)
## Require full C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/third-party)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

project(privacy-service LANGUAGES CXX)

set(Boost_NO_BOOST_CMAKE ON)

find_package(Boost REQUIRED COMPONENTS random system thread filesystem chrono atomic date_time regex unit_test_framework)
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(OPENSSL  REQUIRED COMPONENTS  crypto)
if(OPENSSL_FOUND)
    message(STATUS "OPENSSL_INCLUDE_DIRS: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
    message(STATUS "OPENSSL_VERSION: ${OPENSSL_VERSION}")
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

## Might modify the following path to NTL according to your environment
set(HOMEBREW_INCLUDE /usr/local/include)
set(HOMEBREW_LIB /usr/local/lib)
include_directories(${HOMEBREW_INCLUDE})
link_directories(${HOMEBREW_LIB})

## include third-parties
include_directories(${CMAKE_SOURCE_DIR}/third-party/include)
link_directories(${CMAKE_SOURCE_DIR}/third-party/lib)
## include sqlite
include_directories(${CMAKE_SOURCE_DIR}/third-party/sqlite)

find_package(SEAL 3.4 REQUIRED)

set(helib_DIR ${CMAKE_SOURCE_DIR}/third-party/share/cmake/helib)
find_package(helib 1.0.0 EXACT REQUIRED)

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")
# If you only include this third party in PRIVATE source files, you do not
# need to install it when your main project gets installed.
# set(JSON_Install OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.
add_subdirectory(third-party/nlohmann_json)

# compile lib and binary
add_subdirectory(src)


#include our own unittests
enable_testing()
add_subdirectory(test)
