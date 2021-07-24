# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# sources list
set(source main.cpp)

# config executable
add_executable       (${name} ${source})
source_group         (TREE "${PROJECT_SOURCE_DIR}" FILES ${sources})
set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}")
set_target_properties(${name} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${name} PRIVATE far)
