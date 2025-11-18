# Kernel
add_library(kvlite_core
    core/kvstore.c
)
target_include_directories(kvlite_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# Console client (local, without a network)
add_executable(kvlite_cli
    client/main.c
)
target_link_libraries(kvlite_cli kvlite_core)

# TCP-server
add_executable(kvlite_server
    server/server.c
)
target_link_libraries(kvlite_server kvlite_core)

# Remote CLI-client (network)
add_executable(kvlite_client
    tools/kvlite_client.c
)
target_link_libraries(kvlite_client)

# GUI-client (if added)
if(WIN32)
    add_executable(kvlite_gui
        gui/KVliteGUI.cpp
    )
    target_link_libraries(kvlite_gui ws2_32)
    set_target_properties(kvlite_gui PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Connect Winsock (ws2_32) to ALL network applications
if(WIN32)
    target_link_libraries(kvlite_server ws2_32)
    target_link_libraries(kvlite_client ws2_32)
endif()