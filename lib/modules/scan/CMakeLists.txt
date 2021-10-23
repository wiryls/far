# use dir name as target name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

# collect all header files
set(headers
    include/far/scan.hpp)

# build as a library
add_library(${name} INTERFACE ${headers})

# target options
source_group              (TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${headers})
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}/modules")
set_target_properties     (${name} PROPERTIES CXX_STANDARD 23)
target_include_directories(${name} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options    (${name} INTERFACE
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>"
    "$<$<AND:$<CONFIG:RELEASE>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3>"
    "$<$<AND:$<CONFIG:DEBUG>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-g>"

    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CONFIG:RELEASE>,$<CXX_COMPILER_ID:MSVC>>:/O2>")
