# CMakeList.txt: CMake-Projekt für "PrivacyService". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

project ("PrivacyService" LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
## Use -std=c++11 as default.
set(CMAKE_CXX_STANDARD 17)
## Disable C++ extensions
set(CMAKE_CXX_EXTENSIONS OFF)
## Require full C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#add third-party to cmake prefix
set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/third-party/lib/cmake)

set (RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set (ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set (PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
#QT Moc'ing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)


find_package(Qt5 COMPONENTS Core Network WebSockets Sql REQUIRED)
if(Qt5_FOUND)
    message(STATUS "QT_DIR: ${Qt5_DIR}")
    message(STATUS "QT_VERSION: ${Qt5_VERSION}")
    include_directories(${Qt5_INCLUDE_DIR})
endif()

find_package(Qt5HttpServer)


# Download and unpack seal at configure time
configure_file(third-party/seal.CMakeLists.txt.in seal-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/seal-download )
if(result)
  message(FATAL_ERROR "CMake step for seal failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/seal-download )
if(result)
  message(FATAL_ERROR "Build step for seal failed: ${result}")
endif()

# Add seal directly to our build. 
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/seal-src
                 ${CMAKE_CURRENT_BINARY_DIR}/seal-build
                  )


## include third-parties
include_directories(${CMAKE_SOURCE_DIR}/third-party/include)
link_directories(${CMAKE_SOURCE_DIR}/third-party/lib)


# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_subdirectory(src)


#include our own unittests
#temporarily disabled
#add_subdirectory(test)