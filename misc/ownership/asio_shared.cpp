#include <asio.hpp>
using tcp = asio::ip::tcp;

struct writer : std::enable_shared_from_this<writer> {
    writer(tcp::socket&& socket, std::string message)
        : socket(std::move(socket))
        , message(std::move(message))
    { }

    void keep_writing() {
        asio::async_write(socket,
                          asio::buffer(message, message.size()),
            [this, _(shared_from_this())](std::error_code error,
                                          std::size_t)
            {
                if (not error) keep_writing();
            });
    }

    tcp::socket socket;
    std::string message;
};

int main() {
}
