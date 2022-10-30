#file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
set(SRC_FILES  rest_handler.cpp
            he_controller.cpp
            db_access.cpp
            seal_handler.cpp
            he_controller.cpp
            vicinity_handler.cpp
)

add_library( ${PROJECT_NAME}lib STATIC ${SRC_FILES} )

target_link_libraries(${PROJECT_NAME}lib PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
    Qt6::Sql
    SEAL::seal
)

set_target_properties(${PROJECT_NAME}lib PROPERTIES DEBUG_POSTFIX "d")

#qt_import_plugins(${PROJECT_NAME}lib
#    INCLUDE_BY_TYPE sqldrivers Qt5::QSqlite
#)

add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME}lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION bin
)


#build the plugins
add_subdirectory(mpc_plugin)