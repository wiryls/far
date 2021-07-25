# require 3.11 for FetchContent and 3.14 for FetchContent_MakeAvailable
# see: https://cmake.org/cmake/help/latest/module/FetchContent.html
cmake_minimum_required(VERSION 3.14)
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# helper
# this part is used to collect all third party targets
# https://stackoverflow.com/a/25785856
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(collect_third_party ON)
    set(third_party_targets "" CACHE INTERNAL "third party target list")
    function(add_library name)
        _add_library(${name} ${ARGN})
        remember_target(${name})
    endfunction()
    function(add_executable name)
        _add_executable(${name} ${ARGN})
        remember_target(${name})
    endfunction()
    function(add_custom_target name)
        _add_custom_target(${name} ${ARGN})
        remember_target(${name})
    endfunction()
    function(remember_target name)
        if(collect_third_party AND TARGET ${name} AND NOT ${name} IN_LIST third_party_targets)
            # see: https://cmake.org/cmake/help/latest/command/get_target_property.html
            get_target_property(_aliased  ${name} ALIASED_TARGET)
            get_target_property(_imported ${name} IMPORTED)
            get_target_property(_folder   ${name} FOLDER)
            if(NOT _aliased AND NOT _imported AND NOT _folder STREQUAL "-NOTFOUND")
                set(third_party_targets ${third_party_targets} ${name} CACHE INTERNAL "third party target list")
            endif()
        endif()
    endfunction()
endif()

# third party libraries
#


# populate
FetchContent_MakeAvailable()

# ide friendly
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_target_properties(${third_party_targets} PROPERTIES FOLDER "third_party")
    set(collect_third_party OFF)
endif()
