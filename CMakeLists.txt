cmake_minimum_required(VERSION 3.0)
project(HE-test)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
## Use -std=c++11 as default.
set(CMAKE_CXX_STANDARD 11)

#find_path(CPPREST_INCLUDE cpprest)

find_package(Boost REQUIRED COMPONENTS random system thread filesystem chrono atomic date_time regex)
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
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

## include FHElib from third-party
include_directories(${CMAKE_SOURCE_DIR}/third-party/HElib/include)
link_directories(${CMAKE_SOURCE_DIR}/third-party/HElib/lib)

set(SOURCE  src/main.cpp
            src/rest_handler.cpp
            src/he_handler.cpp
            src/he_controller.cpp)
add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} 
	cpprest
  	Boost::boost
  	Boost::random
  	Boost::system
  	Boost::thread
  	Boost::filesystem
  	Boost::chrono
  	Boost::atomic
  	Boost::date_time
  	Boost::regex
  	OpenSSL::SSL
    crypto
    fhe
    ntl
    gmp
    m
)

##target_link_libraries(${PROJECT_NAME} fhe ntl gmp m)