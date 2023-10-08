FILE(GLOB sources *.cpp)
FILE(GLOB includes include/${PROJECT_NAME}/*)

add_library(${PROJECT_NAME} OBJECT)
target_include_directories(${PROJECT_NAME} PUBLIC include/)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_compile_options(
    ${PROJECT_NAME} BEFORE PRIVATE
    -Wall
    -fPIC
    $<$<CONFIG:DEBUG>:-g>
)

target_sources(${PROJECT_NAME} PRIVATE ${sources})
target_sources(
   ${PROJECT_NAME} PUBLIC FILE_SET HEADERS
   BASE_DIRS include/
   FILES ${includes}
)

configure_file(openmeat.pc.in pkgconfig/openmeat.pc @ONLY)

add_library(${PROJECT_NAME}_shared SHARED)
target_link_libraries(${PROJECT_NAME}_shared PUBLIC ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

add_library(${PROJECT_NAME}_static STATIC)
target_link_libraries(${PROJECT_NAME}_static PUBLIC ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} FILE_SET HEADERS)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/openmeat.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
