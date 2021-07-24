#[[ requirements ]]
cmake_minimum_required(VERSION 3.14)                # for FetchContent_MakeAvailable
enable_testing()                                    # setup for tests.
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # prevent CTest added targets
                                                    # see: https://stackoverflow.com/questions/44949354
include(FetchContent)
set(FETCHCONTENT_QUIET ON)

#[[ dependencies ]]

# catch2 for unit test
FetchContent_Declare(
    catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG        v3.0.0-preview3
    GIT_SHALLOW    ON)

# enable all dependencies
FetchContent_MakeAvailable(catch)
set_target_properties     (Catch2 Catch2WithMain PROPERTIES FOLDER "third_party/tests")

# ignore warnings
foreach(_target Catch2 Catch2WithMain)
    get_target_property(_includes ${_target} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${_target} SYSTEM INTERFACE ${_includes})    
endforeach()

#[[ subprojects ]]

# shared settings for subprojects
add_compile_options(
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-pedantic-errors;-Wall;-Wextra;-Wconversion;-Werror>"
    "$<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:RELEASE>>:-O3>"
    "$<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:DEBUG>>:-g>"

    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CONFIG:RELEASE>,$<CXX_COMPILER_ID:MSVC>>:/O2>")

# add all subprojects
add_subdirectory( benchmark_tests)
add_subdirectory(functional_tests)
