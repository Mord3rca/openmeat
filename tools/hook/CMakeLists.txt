# Minimal hook used to redirect through a proxy
project(hook)

add_library(${PROJECT_NAME} SHARED)

add_compile_options(
    -Wall
    -03
    -D_GNU_SOURCE
    $<$<CONFIG:DEBUG>:-g>
)

target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hook.c)
target_link_libraries(${PROJECT_NAME} dl)

install(TARGETS ${PROJECT_NAME} LIBRARY
        DESTINATION "${CMAKE_INSTALL_DATADIR}/${GLOBAL_NAME}"
)
