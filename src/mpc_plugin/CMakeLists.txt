set(SRC_FILES  
    mpc_plugin.cpp
)

add_library( mpc_plugin MODULE ${SRC_FILES} )

target_link_libraries(mpc_plugin PRIVATE
    ${PROJECT_NAME}_lib
)