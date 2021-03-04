#file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
set(SRC_FILES  rest_handler.cpp
            he_controller.cpp
            db_access.cpp
            seal_handler.cpp
            he_controller.cpp
            vicinity_handler.cpp
)

add_library( ${PROJECT_NAME}_lib OBJECT ${SRC_FILES} )

target_link_libraries(${PROJECT_NAME}_lib PUBLIC
    Qt5::Core
    Qt5::HttpServer
    Qt5::Sql
    SEAL::seal
)

#qt_import_plugins(${PROJECT_NAME}_lib
#    INCLUDE_BY_TYPE sqldrivers Qt5::QSqlite
#)

add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries(${PROJECT_NAME} 
    ${PROJECT_NAME}_lib
)


#build the plugins
add_subdirectory(mpc_plugin)