project(server)
gcpt_shared_set_source_groups()

set(SOURCES
    main.cpp
)

set(HEADERS
    
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} gcpt::shared)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME gcpt_server VS_DEBUGGER_COMMAND_ARGUMENTS "-fileroot \"${CMAKE_SOURCE_DIR}\"")
target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_SERVER=1)

get_target_property(GCPT_SHARED_SRC gcpt::shared INTERFACE_SOURCES)
set(GCPT_SHARED_SOURCES ${GCPT_SHARED_SRC})
set(GCPT_SHARED_HEADERS ${GCPT_SHARED_SRC})

list(FILTER GCPT_SHARED_SOURCES EXCLUDE REGEX ".*\\.(h|hpp)")
list(FILTER GCPT_SHARED_HEADERS INCLUDE REGEX ".*\\.(h|hpp)")
