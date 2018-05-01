cmake_minimum_required(VERSION 3.9)

project(cppnow2018_full_duplex_project)
enable_testing()

#include(quick_project/Test)
#quick_project_add_check_target()

add_definitions(-std=c++1z)
add_definitions(-ftemplate-backtrace-limit=0)
add_definitions(-Wall)
add_definitions(-Wextra)
add_definitions(-DBOOST_ASIO_NO_TYPEID -DBOOST_ASIO_HAS_STD_STRING_VIEW)

include_directories(${cppnow2018_full_duplex_project_SOURCE_DIR}/include)

add_subdirectory(example)
#add_subdirectory(test)
