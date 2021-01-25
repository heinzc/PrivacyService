ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
enable_testing()

# Download and unpack googletest at configure time
configure_file(../third-party/googletest.CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)


file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)
#set(TEST_SRC_FILES  test_main.cpp
#            test_base64.cpp
#)

add_executable(${PROJECT_NAME}_unittest ${TEST_SRC_FILES})

target_link_libraries(${PROJECT_NAME}_unittest 
        ${PROJECT_NAME}_lib
        Boost::unit_test_framework
        gtest_main
)

add_test(NAME UnitTests COMMAND ${PROJECT_NAME}_unittest)