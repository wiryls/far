# use dir name as target name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# collect all header files and source files
set(headers
    include/far/exec.hpp
    include/far/stat.hpp
    include/far/task.hpp)
set(sources
    src/exec.cpp
    src/stat.cpp)

# build as a library
add_library(${name} STATIC ${sources} ${headers})

# target options
source_group              (TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"     PREFIX "Source Files" FILES ${sources})
source_group              (TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${headers})
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}/modules")
set_target_properties     (${name} PROPERTIES CXX_STANDARD 23)
target_include_directories(${name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries     (${name} PRIVATE "$<$<CXX_COMPILER_ID:GNU>:pthread>")
target_compile_options    (${name} PRIVATE 
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>"
    "$<$<AND:$<CONFIG:RELEASE>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3>"
    "$<$<AND:$<CONFIG:DEBUG>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-g>"

    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CONFIG:RELEASE>,$<CXX_COMPILER_ID:MSVC>>:/O2>")

# Note:
# CXX_STANDARD 23 is used to enable "/std:c++latest"
# https://stackoverflow.com/questions/64889383
