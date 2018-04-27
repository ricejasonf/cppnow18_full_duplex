#include <iostream>

struct error {
    char const* message;
};

decltype(auto) operator<< (std::ostream& os, error e) {
    os << "error(\"" << e.message << "\")";
    return os;
}

int main() {

auto foo = [](auto resolve) { return [=](auto x) {
    resolve(x + x);
};};

auto bar = [](auto resolve) { return [=](auto x) {
    if (x < 50) {
        resolve(x);
    } else {
        resolve(error{"out of range"});
    }
};};

auto baz = [](auto resolve) { return [=](auto x) {
    std::cout << x << '\n';
    resolve(x);
};};

auto noop = [](auto) { };

foo(
bar(
baz(
 noop
))) (5);

foo(
bar(
baz(
 noop
))) (42);

}
