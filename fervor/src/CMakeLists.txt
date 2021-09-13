# set the target name
set(name "far")

# collect all header files and source files
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp")
set(sources
    "exec.cpp"
    "task.cpp")

# build as a library
add_library(${name} ${sources} ${headers})

# target options
source_group              (TREE "${PROJECT_SOURCE_DIR}" FILES ${headers})
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}")
set_target_properties     (${name} PROPERTIES CXX_STANDARD 23)
target_include_directories(${name} PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries     (${name} PRIVATE "$<$<CXX_COMPILER_ID:GNU>:pthread>")
target_compile_options    (${name} PRIVATE 
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>"
    "$<$<AND:$<CONFIG:RELEASE>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3>"
    "$<$<AND:$<CONFIG:DEBUG>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-g>"

    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CONFIG:RELEASE>,$<CXX_COMPILER_ID:MSVC>>:/O2>")

# Note:
# CXX_STANDARD 23 is used to enable "/std:c++latest" https://stackoverflow.com/questions/64889383
