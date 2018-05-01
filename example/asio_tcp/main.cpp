//
// Copyright Jason Rice 2018
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <asio_tcp.hpp>
#include <full_duplex.hpp>
#include <iostream>

int main() {
    asio::io_service io;

    using full_duplex::endpoint_open;
    using full_duplex::endpoint_compose;

    std::queue<std::string> messages{std::deque<std::string>{
        "message1",
        "message2",
        "message3",
        "message4",
        "message5"
    }};

    // listener
    endpoint_open(
        asio_tcp::make_accept_state(io, 9081),
        std::queue<std::string>{},
        endpoint_compose(
            asio_tcp::accept_one,
            asio_tcp::raw_message,
            endpoint(
                event::read_message = [](auto& self) {
                    return tap([&](auto& msg) {
                        // echo the message
                        self.send_message(msg);
                    });
                }
                event::error = [](auto& self) {
                    return tap([&](auto&& error) {
                        std::cout << "LISTENER ERROR: " << msg.message() << " \n";
                    });
                }
            )
        )
    );

    auto sender = endpoint_open(
        asio_tcp::make_connect_state(io, 9081),
        std::move(messages)
        endpoint_compose(
            asio_tcp::accept_one,
            asio_tcp::raw_message,
            endpoint(
                event::error = [](auto& self) {
                    return tap([&](auto&& error) {
                        std::cout << "SENDER ERROR: " << msg.message() << " \n";
                    });
                }
            )
        )
    );

    send_message(sender, std::string("message6"));

    io.run();
}
