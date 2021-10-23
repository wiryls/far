# use dir name as name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# sources list
set(headers
    include/far/task.hpp)
set(sources
    src/main.cpp)

# config executable
add_library               (${name} SHARED ${sources} ${headers})
source_group              (TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"     PREFIX "Source Files" FILES ${sources})
source_group              (TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${headers})
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}/exports")
set_target_properties     (${name} PROPERTIES CXX_STANDARD 23)
target_include_directories(${name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(${name} PRIVATE FARTASK_LIBRARY_EXPORTS)
target_link_libraries     (${name} PRIVATE gear scan)
