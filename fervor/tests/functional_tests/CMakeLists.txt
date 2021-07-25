# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# source lists
set(sources
    faregex_test.cpp)

# executable
add_executable       (${name} ${sources})
set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/tests")
set_target_properties(${name} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${name} PRIVATE Catch2::Catch2WithMain far)

# add for CTest
add_test(NAME ${name} COMMAND ${name})
