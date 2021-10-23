# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# sources list
set(source main.cpp)

# config executable
add_executable       (${name} ${source})
source_group         (TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})
set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/exports")
set_target_properties(${name} PROPERTIES CXX_STANDARD 23)
target_link_libraries(${name} PRIVATE gear scan)
