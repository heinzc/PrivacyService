# CMakeList.txt: CMake-Projekt für "PrivacyService". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.20)

project ("PrivacyService" LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
## Use -std=c++11 as default.
set(CMAKE_CXX_STANDARD 17)
## Disable C++ extensions
set(CMAKE_CXX_EXTENSIONS OFF)
## Require full C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#options
option(PRIVACYSERVICE_TESTS "compile and run unittests" OFF)

#set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
# set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
# set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib)
# set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib)
# set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib)
# set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib)
# set (CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
#QT Moc'ing
set(CMAKE_AUTOMOC ON)
#No Qt Ressources used
set(CMAKE_AUTORCC OFF)
#No Qt UI files used
set(CMAKE_AUTOUIC OFF)


find_package(Qt6 COMPONENTS Core Network HttpServer Sql Test REQUIRED)
if(Qt6_FOUND)
    message(STATUS "QT_DIR: ${Qt6_DIR}")
    message(STATUS "QT_VERSION: ${Qt6_VERSION}")
endif()

#find_package(Qt5HttpServer)

#find_package(SEAL 3.6 QUIET)
#if(SEAL_FOUND)
#    message(STATUS "Using lokal Setup of SEAL, Version: ${SEAL_VERSION}")
#    set(SEAL_USE_LOCAL ON CACHE BOOL "if a local seal install should be used")
#endif()

#if(NOT SEAL_USE_LOCAL)
    message(STATUS "No lokal SEAL found. Including it in build.")

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
#endif()


# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_subdirectory(src)

if(PRIVACYSERVICE_TESTS)
  add_subdirectory(test)
endif()