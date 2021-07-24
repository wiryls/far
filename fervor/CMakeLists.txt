# cmake requirement
cmake_minimum_required(VERSION 3.19...3.22)

# project infomation
project(fervor
    VERSION 0.0.1
    DESCRIPTION "fervor of FAR"
    LANGUAGES CXX
)

# provide options for users
option(UNCAT_BUILD_TESTS    "build all test programs"   OFF)
option(UNCAT_BUILD_EXAMPLES "build all sample programs" OFF)

# global properties or options
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "")

# add their and our libraries
add_subdirectory(third_party)
add_subdirectory(src)

# add executable
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND UNCAT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# add tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND UNCAT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
