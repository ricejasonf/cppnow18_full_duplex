add_executable(example.beast_ws main.cpp)

target_link_libraries("example.beast_ws"
    ${CMAKE_THREAD_LIBS_INIT}
)

add_custom_target(run.example.beast_ws
    COMMAND ./example.beast_ws
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Build and run: example.beast_ws"
)
add_dependencies(run.example.beast_ws example.beast_ws)
