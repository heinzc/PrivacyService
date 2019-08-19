cmake_minimum_required(VERSION 3.2.2)
project(third-party)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ".cmake")

set(HOMEBREW_INCLUDE /usr/local/include)
set(HOMEBREW_LIB /usr/local/lib)
include_directories(${HOMEBREW_INCLUDE})
link_directories(${HOMEBREW_LIB})

add_subdirectory(HElib)
add_subdirectory(libhcs)
