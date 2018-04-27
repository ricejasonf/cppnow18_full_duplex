#include <iostream>
#include <string>

auto make_handler() {
    std::string msg = "Hello, world!\n";
    return [&] { std::cout << msg; };
}

int main() {
    auto handle = make_handler();

    handle();
}
