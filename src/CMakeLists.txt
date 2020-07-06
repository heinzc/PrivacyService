file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
set(SRC_FILES  rest_handler.cpp
            fhe_handler.cpp
            phe_handler.cpp
            he_controller.cpp
            db_access.cpp
            seal_he_handler.cpp
            he_controller.cpp
            vicinity_handler.cpp
            base64.cpp
            ../third-party/sqlite/shell.c
            ../third-party/sqlite/sqlite3.c
)

add_library(${PROJECT_NAME}_lib ${SRC_FILES})
target_link_libraries(${PROJECT_NAME}_lib
    nlohmann_json::nlohmann_json
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
    OpenSSL::Crypto
    SEAL::seal
    #crypto
    #ssl
    helib
    hcs
    ntl
    gmp
    m
    sqlite3
)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)