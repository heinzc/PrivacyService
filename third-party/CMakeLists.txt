cmake_minimum_required(VERSION 3.2.2)
project(third-party)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "where third parties will be installed" FORCE)


set(HOMEBREW_INCLUDE /usr/local/include)
set(HOMEBREW_LIB /usr/local/lib)
include_directories(${HOMEBREW_INCLUDE})
link_directories(${HOMEBREW_LIB})


# Path containing FindGMP.cmake and FindNTL.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/HElib/cmake")
add_subdirectory(HElib)


# Path containing FindGMP.cmake and FindNTL.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libhcs/.cmake")
add_subdirectory(libhcs)
