enable_testing(true)


set(TEST_SRC_FILES  #test_main.cpp
            test_db_access.cpp
)


add_executable(${PROJECT_NAME}_unittest ${TEST_SRC_FILES})


target_link_libraries(${PROJECT_NAME}_unittest PRIVATE
        ${PROJECT_NAME}_lib
        Qt5::Test
)


add_test(NAME UnitTests COMMAND ${PROJECT_NAME}_unittest)