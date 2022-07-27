set(SRC_FILES  
    svm_plugin.cpp
    SVM_Handler.cpp
    SVM_Task.cpp
    Recrypt_Task.cpp
)
find_package(Threads)

add_library( svm_plugin MODULE ${SRC_FILES})

target_link_libraries(svm_plugin PRIVATE
    ${PROJECT_NAME}_lib
    Threads::Threads
)