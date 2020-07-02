ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

set(SOURCE  test_main.cpp
            test_base64.cpp
)

add_executable(${PROJECT_NAME}_unittest ${SOURCE})

target_link_libraries(${PROJECT_NAME}_unittest 
        ${PROJECT_NAME}_lib
        Boost::unit_test_framework
)

add_test(NAME UnitTests COMMAND ${PROJECT_NAME}_unittest)