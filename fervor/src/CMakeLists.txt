# set the target name
set(name "far")

# collect all header files and source files
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp")

# build as a library
add_library(${name} INTERFACE ${headers})

# target options
source_group              (TREE "${PROJECT_SOURCE_DIR}" FILES ${headers})
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}")
set_target_properties     (${name} PROPERTIES CXX_STANDARD 20)
target_include_directories(${name} INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries     (${name} INTERFACE "$<$<CXX_COMPILER_ID:GNU>:pthread")
target_compile_options    (${name} INTERFACE
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>"
    "$<$<AND:$<CONFIG:RELEASE>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3>"
    "$<$<AND:$<CONFIG:DEBUG>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-g>"

    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CONFIG:RELEASE>,$<CXX_COMPILER_ID:MSVC>>:/O2>")
