FILE(GLOB sources *.cpp)
FILE(GLOB includes include/${PROJECT_NAME}/*)

add_library(${PROJECT_NAME} SHARED)

target_compile_options(
    ${PROJECT_NAME} BEFORE PRIVATE
    -Wall
    $<$<CONFIG:DEBUG>:-g>
)

configure_file(openmeat.pc.in pkgconfig/openmeat.pc @ONLY)

target_include_directories(${PROJECT_NAME} PUBLIC include/)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_sources(${PROJECT_NAME} PRIVATE ${sources})

target_sources(
   ${PROJECT_NAME} PUBLIC FILE_SET HEADERS
   BASE_DIRS include/
   FILES ${includes}
)

install(TARGETS ${PROJECT_NAME} LIBRARY FILE_SET HEADERS)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/openmeat.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
