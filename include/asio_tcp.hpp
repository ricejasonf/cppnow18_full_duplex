//
// Copyright Jason Rice 2018
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPNOW18_FULL_DUPLEX_ASIO_TCP_HPP
#define CPPNOW18_FULL_DUPLEX_ASIO_TCP_HPP

#include <asio.hpp>
#include <cstdint>
#include <full_duplex.hpp>
#include <string>
#include <functional>
#include <utility>

namespace asio_tcp {
    using asio::ip::tcp;
    using full_duplex::do_;
    using full_duplex::endpoint;
    using full_duplex::make_error;
    using full_duplex::promise;
    namespace event = full_duplex::event;

    // accept

    struct accept_state {
        accept_state(asio::io_service& io, unsigned short port)
            : socket(io)
            , endpoint(tcp::v4(), port)
            , accecptor(io)
        { }

        tcp::socket socket;
        tcp::endpoint endpoint;
        tcp::acceptor acceptor;
    };

    constexpr auto accept = [](auto&) {
        return promise([](auto& resolve, auto& self) {
            auto& state = self.state();
            state.acceptor.bind(endpoint);
            state.acceptor.async_accept(state.socket, [&](auto error) {
                (not error) ? resolve(self) : resolve(make_error(error));
            });
        });
    };

    // connect

    struct connect_state {
        accept_state(asio::io_service& io, unsigned short port)
            : socket(io)
            , endpoint(tcp::v4(), port)
        { }

        tcp::socket socket;
        tcp::endpoint;
    };

    constexpr auto make_connect_state = [](asio::io_service& io, auto port) {
        return accept_state{
            tcp::socket(io),
            tcp::endpoint(tcp::v4(), port)
        };
    };

    constexpr auto connect = [](auto&) {
        return promise([](auto& resolve, auto& self) {
            auto& state = self.state();
            state.socket.async_connect(state.endpoint, [&](auto error) {
                (not error) ? resolve(self) : resolve(make_error(error));
            });
        });
    };

    // read message

    struct read_length_fn {
        template <typename Resolve, typename Self>
        auto operator()(Resolve& resolve, Self& self) {
            asio::async_read(self.state().socket, asio_buffer(buffer, 4), [&](auto error, size_t) {
                uint_32_t length = 
                      buffer[0] << 24
                    | buffer[1] << 16
                    | buffer[2] << 8
                    | buffer[3];

                (not error) ? resolve(std::pair(std::ref(socket), length)) : resolve(make_error(error));
            });
        }

        unsigned char buffer[4];
    };

    struct read_body_fn {
        template <typename Resolve, typename Pair>
        auto operator()(Resolve& resolve, Pair const& pair) {
            auto& [socket_ref, length] = pair;
            asio::async_read(socket_ref.get(), asio_buffer(body, length), [&](auto error, size_t) {
                (not error) ? resolve(body) : resolve(make_error(error));
            });
        }

        std::string body;
    };

    constexpr auto read_message = [](auto&) {
        return do_(
            promise(read_length_fn{}),
            promise(read_body_fn{})
        );
    };


    // write_message

    struct write_length_fn {
        template <typename Resolve, typename Message>
        auto operator()(Resolve& resolve, Message& m) {
			buffer[0] = static_cast<unsigned char>(m.size() >> 24);
			buffer[1] = static_cast<unsigned char>(m.size() >> 16);
			buffer[2] = static_cast<unsigned char>(m.size() >>  8);
			buffer[3] = static_cast<unsigned char>(m.size());

            asio::async_write(socket, asio_buffer(buffer, 4), [&](auto error, size_t) {
                (not error) ? resolve(message) : resolve(make_error(error));
            });
        }

		tcp::socket socket;
        unsigned char buffer[4] = {};
    };

    constexpr auto write_message = [](auto& self) {
        return do_(
            promise(write_length_fn{self.state().socket}),
            promise([&](auto& resolve, auto& message) {
                asio::async_write(self.state().socket, asio::buffer(message, message.size()),
                    [&](auto error, size_t) {
                        (not error) ? resolve(message) : resolve(make_error(error));
					}
                );
            })
        );
    };

    // endpoint "classes"

    constexpr auto accept_one = endpoint(event::init = accept);
    constexpr auto connect    = endpoint(event::init = connect);

    constexpr auto raw_message = endpoint(
        event::read_message  = read_message,
        event::write_message = write_message
    );
}

#endif
