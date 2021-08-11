# cmake requirement
cmake_minimum_required(VERSION 3.19...3.22)

# project infomation
project(far
    VERSION 0.0.1
    DESCRIPTION "find and rename"
    LANGUAGES CXX)

# provide options for users
option(FAR_BUILD_TESTS    "build all test programs"   ON)
option(FAR_BUILD_EXAMPLES "build all sample programs" ON)

# global properties or options
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "")

# add their and our libraries
add_subdirectory(third_party)
add_subdirectory(src)

# add executable
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND FAR_BUILD_EXAMPLES)
    add_subdirectory(apps)
endif()

# add tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND FAR_BUILD_TESTS)
    add_subdirectory(tests)
endif()
