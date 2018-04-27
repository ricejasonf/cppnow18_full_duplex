
constexpr auto my_endpoint = endpoint(
    event::init          = [](auto& state) {
        return tap([&](auto&&) {
            std::cout << "User session started: "
                      << state.session_name << '\n';
        });
    },
    event::read_message  = [](auto&) { 
        return tap([](auto&& message) {
            std::cout << "Message received: "
                      << message << '\n';
        });
    },
    event::write_message = [](auto&) {
        return do_();
    }
);

struct my_state {
    tcp::socket socket;
    std::string session_name = {};
};

auto ep = endpoint_open(
    my_state{socket},
    std::queue<std::string>{},
    endpoint_compose(beast_ws_client, my_endpoint)
);
