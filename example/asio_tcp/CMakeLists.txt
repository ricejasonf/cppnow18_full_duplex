add_executable(example.asio_tcp main.cpp)

target_link_libraries("example.asio_tcp"
    ${CMAKE_THREAD_LIBS_INIT}
)

add_custom_target(run.example.asio_tcp
    COMMAND ./example.asio_tcp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Build and run: example.asio_tcp"
)
add_dependencies(run.example.asio_tcp example.asio_tcp)
