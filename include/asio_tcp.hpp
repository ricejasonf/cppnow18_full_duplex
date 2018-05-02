//
// Copyright Jason Rice 2018
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPNOW18_FULL_DUPLEX_ASIO_TCP_HPP
#define CPPNOW18_FULL_DUPLEX_ASIO_TCP_HPP

#include <boost/asio.hpp>
#include <cstdint>
#include <full_duplex.hpp>
#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace asio_tcp {
    namespace asio = boost::asio;
    using asio::ip::tcp;
    using full_duplex::do_;
    using full_duplex::endpoint;
    using full_duplex::make_error;
    using full_duplex::promise;
    namespace event = full_duplex::event;

    // accept

    struct accept_state {
        accept_state(asio::io_service& io, unsigned short port)
            : socket_(io)
            , endpoint(tcp::v4(), port)
            , acceptor()
        { }

        tcp::socket& socket()
        { return socket_; }

        tcp::socket socket_;
        tcp::endpoint endpoint;
        std::optional<tcp::acceptor> acceptor;
    };

    constexpr auto accept = [](auto& self) {
        return promise([&](auto& resolve, auto&&) {
            auto& state = self.state();
            auto acceptor = tcp::acceptor(state.socket().get_io_service(), state.endpoint);
            acceptor.async_accept(state.socket(), [&](auto error) {
                (not error) ? resolve(self) : resolve(make_error(error));
            });
            state.acceptor = std::move(acceptor);
        });
    };

    // connect

    struct connect_state {
        connect_state(asio::io_service& io, unsigned short port)
            : socket_(io)
            , endpoint(tcp::v4(), port)
        { }

        tcp::socket& socket()
        { return socket_; }

        tcp::socket socket_;
        tcp::endpoint endpoint;
    };

    constexpr auto connect = [](auto& self) {
        return promise([&](auto& resolve, auto&&) {
            auto& state = self.state();
            state.socket().async_connect(state.endpoint, [&](auto error) {
                (not error) ? resolve(self) : resolve(make_error(error));
            });
        });
    };

    // read message

    template <typename Self>
    struct read_length_fn {
        template <typename Resolve, typename Input>
        auto operator()(Resolve& resolve, Input&&) {
            asio::async_read(self.state().socket(), asio::buffer(buffer, 4), [&](auto error, size_t) {
                uint32_t length = buffer[0] << 24
                                | buffer[1] << 16
                                | buffer[2] << 8
                                | buffer[3];
                (not error) ? resolve(length) : resolve(make_error(error));
            });
        }

        template <typename S>
        explicit read_length_fn(S& s)
            : self(s)
            , buffer()
        { }

        Self& self;
        unsigned char buffer[4];
    };

    template <typename Self>
    struct read_body_fn {
        template <typename Resolve>
        auto operator()(Resolve& resolve, uint32_t length) {
            body.resize(length);
            asio::async_read(self.state().socket(), asio::buffer(body, length), [&](auto error, size_t) {
                (not error) ? resolve(body) : resolve(make_error(error));
            });
        }

        explicit read_body_fn(Self& s)
            : self(s)
            , body()
        { }

        Self& self;
        std::string body;
    };

    constexpr auto read_message = [](auto& self) {
        using Self = decltype(self);
        return do_(
            promise(read_length_fn<Self>(self)),
            promise(read_body_fn<Self>{self})
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

            asio::async_write(socket, asio::buffer(buffer, 4), [&](auto error, size_t) {
                (not error) ? resolve(m) : resolve(make_error(error));
            });
        }

		tcp::socket& socket;
        unsigned char buffer[4] = {};
    };

    constexpr auto write_message = [](auto& self) {
        tcp::socket& socket = self.state().socket();

        return do_(
            promise(write_length_fn{socket}),
            promise([&](auto& resolve, auto& message) {
                asio::async_write(socket, asio::buffer(message, message.size()),
                    [&](auto error, size_t) {
                        (not error) ? resolve(message) : resolve(make_error(error));
					}
                );
            })
        );
    };

    // endpoint "classes"

    constexpr auto acceptor   = endpoint(event::init = accept);
    constexpr auto connector  = endpoint(event::init = connect);

    constexpr auto message = endpoint(
        event::read_message  = read_message,
        event::write_message = write_message
    );
}

#endif
