project(packet-deadmaze)

find_package(Wireshark)

get_target_property(epan_includes epan INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(epan_system_includes epan INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME} PRIVATE packet-deadmaze.c)

target_include_directories(${PROJECT_NAME} PRIVATE ${epan_includes})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${epan_system_includes})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "deadmaze")

if(Wireshark_PLUGINS_ENABLED)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "${Wireshark_PLUGIN_INSTALL_DIR}/epan/")
endif()
