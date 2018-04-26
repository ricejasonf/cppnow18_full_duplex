//
// Copyright Jason Rice 2018
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef AMQP_CLIENT_HPP
#define AMQP_CLIENT_HPP

#include <amqpcpp.h>
#include <asio.hpp>
#include <memory>

namespace amqp {
using asio::ip::tcp;

namespace amqp::detail
{
    constexpr int no_flags = 0;
    class connection_handler : public AMQP::ConnectionHandler
    {
        std::string host;
        std::string port;

        /**
         *  Method that is called by the AMQP library every time it has data
         *  available that should be sent to RabbitMQ.
         *  @param  connection  pointer to the main connection object
         *  @param  data        memory buffer with the data that should be sent to RabbitMQ
         *  @param  size        size of the buffer
         */
        virtual void onData(AMQP::Connection *connection, const char *data, size_t size)
        {
            asio::async_write(
                socket,
                asio::const_buffer(data, size),
                [](asio::error_code const& ec, std::size_t) {
                    if (ec) {
                        std::cerr << "\nERROR: Socket write failed: " << ec.message() << '\n'; 
                    }
                }
            );
        }

        /**
         *  Method that is called by the AMQP library when the login attempt
         *  succeeded. After this method has been called, the connection is ready
         *  to use.
         *  @param  connection      The connection that can now be used
         */
        virtual void onConnected(AMQP::Connection *connection) {
            std::cout << "onConnected\n";
        }

        /**
         *  Method that is called by the AMQP library when a fatal error occurs
         *  on the connection, for example because data received from RabbitMQ
         *  could not be recognized.
         *  @param  connection      The connection on which the error occured
         *  @param  message         A human readable error message
         */
        virtual void onError(AMQP::Connection *connection, const char *message) {
            std::cerr << "\nRabbitMQ ERROR: " << message << '\n'; 
        }

        /**
         *  Method that is called when the connection was closed. This is the
         *  counter part of a call to Connection::close() and it confirms that the
         *  connection was correctly closed.
         *
         *  @param  connection      The connection that was closed and that is now unusable
         */
        virtual void onClosed(AMQP::Connection *connection)
        { }

        public:

        tcp::socket socket;

        connection_handler(asio::io_service& io, std::string const& host, std::string const& port)
          : host(host)
          , port(port)
          , socket(tcp::socket(io))
        {
            connect();
        }

        void connect() {
          asio::connect(
              socket,
              tcp::resolver(socket.get_io_service()).resolve(tcp::resolver::query(host, port))
          );
        }
    };

    class connection_impl : public std::enable_shared_from_this<connection_impl> {
        tcp::socket &socket;
        char buffer[1024];

        public:

        // just expose the impl layer
        AMQP::Connection amqp_impl;

        connection_impl(connection_handler* handler)
          : socket(handler->socket)
          , amqp_impl(handler, AMQP::Login("guest", "guest"), "/")
          , buffer()
        { }

        // prevent copy because we are holding a reference
        connection_impl(connection_impl const&) = delete;

        void keep_reading() {
            auto self(shared_from_this());
            socket.async_read_some(
                asio::mutable_buffer(buffer, sizeof(buffer)),
                [this, self](asio::error_code const& ec, std::size_t length) {
                    if (!ec) {
                        // send the received data to the amqp layer
                        amqp_impl.parse(buffer, length);
                        keep_reading();
                    }
                    else {
                        std::cerr << "\nERROR: Socket read failed: " << ec.message() << '\n';
                    }
                }
            );
        }
    };
}

namespace amqp
{
    class client {
        detail::connection_handler handler;
        std::shared_ptr<detail::connection_impl> impl;

        public:

        client(asio::io_service& io, std::string const& host, std::string const& port)
          : handler(io, host, port)
          , impl(new detail::connection_impl(&handler))
        {
            impl->keep_reading();
        }

        client(client const&) = delete;

        AMQP::Connection& amqp() {
            return impl->amqp_impl;
        }
    };
}

#endif
