//
// Copyright Jason Rice 2018
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <asio_tcp.hpp>
#include <beast_ws.hpp>

#include <boost/asio.hpp>
#include <full_duplex.hpp>
#include <iostream>
#include <utility>

int main() {
    namespace event = full_duplex::event;
    using full_duplex::endpoint;
    using full_duplex::endpoint_compose;
    using full_duplex::endpoint_open;
    using full_duplex::promise;
    using full_duplex::tap;

    boost::asio::io_service io;

    std::queue<std::string> messages{std::deque<std::string>{
        "message1",
        "message2",
        "message3",
        "message4",
        "message5"
    }};

    // listener
    endpoint_open(
        beast_ws::accept_state{io, 9081},
        std::queue<std::string>{},
        endpoint_compose(
            asio_tcp::acceptor,
            beast_ws::acceptor,
            beast_ws::message,
            endpoint(
                event::read_message = [](auto& self) {
                    return promise([&](auto& resolve, auto const& msg) {
                        if (msg == "terminate") {
                            resolve(full_duplex::terminate{});
                        } 
                        else {
                            // echo the message
                            self.send_message(msg);
                            resolve(msg);
                        }
                    });
                },
                event::error = [](auto&) {
                    return tap([](auto& error) {
                        std::cout << "LISTENER ERROR: " << error.message() << " \n";
                    });
                }
            )
        )
    );

    auto sender = endpoint_open(
        beast_ws::connect_state{io, 9081},
        std::move(messages),
        endpoint_compose(
            asio_tcp::connector,
            beast_ws::connector,
            beast_ws::message,
            endpoint(
                event::read_message = [](auto&) {
                    return tap([](auto& msg) {
                        std::cout << "MESSAGE: " << msg << '\n';
                    });
                },
                event::error = [](auto&) {
                    return tap([](auto&& error) {
                        std::cout << "SENDER ERROR: " << error.message() << " \n";
                    });
                }
            )
        )
    );

    full_duplex::send_message(sender, std::string("message6"));
    full_duplex::send_message(sender, std::string("terminate"));

    io.run();
}
