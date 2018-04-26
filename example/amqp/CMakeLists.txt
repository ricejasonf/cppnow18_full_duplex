#
# Copyright Jason Rice 2018
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_definitions(-std=c++1z)
add_definitions(-DASIO_STANDALONE -DASIO_NO_TYPEID -DASIO_HAS_STD_STRING_VIEW)

find_package(Threads)
find_library(JsonCpp jsoncpp)

add_executable("example.amqp" main.cpp)
target_link_libraries("example.amqp"
  ${JsonCpp}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_custom_target(run.example.amqp
  COMMAND ./example.amqp
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Build and run: example.amqp"
)
add_dependencies(run.example.amqp example.amqp)
