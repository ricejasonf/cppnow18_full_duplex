//
// Copyright Jason Rice 2018
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPNOW18_FULL_DUPLEX_BEAST_WS_HPP
#define CPPNOW18_FULL_DUPLEX_BEAST_WS_HPP

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>
#include <full_duplex.hpp>
#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace beast_ws {
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
            : ws(io)
            , endpoint(tcp::v4(), port)
            , acceptor()
        { }

        tcp::socket& socket()
        { return ws.next_layer(); }

        boost::beast::websocket::stream<tcp::socket> ws;
        tcp::endpoint endpoint;
        std::optional<tcp::acceptor> acceptor;
    };

    constexpr auto accept = [](auto& self) {
        return promise([&](auto& resolve, auto&&) {
            auto& state = self.state();
            state.ws.async_accept([&](auto error) {
                (not error) ? resolve(self) : resolve(make_error(error));
            });
        });
    };

    // connect

    struct connect_state {
        connect_state(asio::io_service& io, unsigned short port)
            : ws(io)
            , endpoint(tcp::v4(), port)
        { }

        tcp::socket& socket()
        { return ws.next_layer(); }

        boost::beast::websocket::stream<tcp::socket> ws;
        tcp::endpoint endpoint;
    };

    constexpr auto send_handshake = [](auto&) {
        return promise([](auto& resolve, auto& self) {
            auto& state = self.state();
            state.ws.async_handshake(boost::string_view("foo"), boost::string_view("/"),
                [&](auto error) {
                    (not error) ? resolve(self) : resolve(make_error(error));
                });
        });
    };

    // read message

    template <typename Self>
    struct read_message_fn {
        template <typename Resolve, typename Input>
        void operator()(Resolve& resolve, Input&&) {
            self.state().ws.async_read(buffer, [&](auto error, size_t) {
                auto mut_buf = buffer.data();
                std::string_view buffer_view(static_cast<char const*>(mut_buf.data()), mut_buf.size());
                body.resize(buffer_view.size());
                std::copy(buffer_view.begin(), buffer_view.end(), body.begin()); // :(
                buffer.consume(buffer.size());
                (not error) ? resolve(body) : resolve(make_error(error));
            });
        }

        explicit read_message_fn(Self& s)
            : self(s)
            , body()
            , buffer()
        { }

        Self& self;
        std::string body;
        boost::beast::flat_buffer buffer;
    };

    constexpr auto read_message = [](auto& self) {
        using Self = decltype(self);
        return promise(read_message_fn<Self>(self));
    };

    // write_message

    constexpr auto write_message = [](auto& self) {
        return promise([&](auto& resolve, auto& message) {
            self.state().ws.async_write(
                asio::buffer(message, message.size()),
                [&](auto error, size_t) {
                    (not error) ? resolve(message) : resolve(make_error(error));
                }
            );
        });
    };

    // endpoint "classes"

    constexpr auto acceptor   = endpoint(event::init = accept);
    constexpr auto connector  = endpoint(event::init = send_handshake);

    constexpr auto message = endpoint(
        event::read_message  = read_message,
        event::write_message = write_message
    );
}

#endif
