//
// Copyright Jason Rice 2018
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "amqp_client.hpp"
#include <asio.hpp>

int main() {
    asio::io_service io;
    amqp::client test_pusher(io, "127.0.0.1", "5672");
    AMQP::Channel publish_channel(&(test_pusher.amqp()));

    amqp::client client(io, "127.0.0.1", "5672");
    auto& amqp = client.amqp();
    AMQP::Channel consumer_channel(&amqp);
    consumer_channel.declareExchange("my_exchange", AMQP::fanout);
    consumer_channel.declareQueue("")
        .onSuccess([&consumer_channel](std::string const& name, uint32_t, uint32_t) {
            consumer_channel.bindQueue("my_exchange", name, "my_key");
        });

    consumer_channel.consume("")
        .onMessage([](AMQP::Message const& msg, uint64_t, bool) {
            std::cout << "RECEIVED MESSAGE: " << msg.message() << "\n";
        })
        .onSuccess([&publish_channel]() {
            std::cout << "consuming successfully\n";
            publish_channel.publish("my_exchange", "my_key", "Hello, Message!");
        })
        .onError([](char const* err) {
            std::cerr << "ERROR: " << err << '\n';
        });
    io.run();
}
