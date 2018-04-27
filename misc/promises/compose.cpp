#include <iostream>

int main() {

auto foo = [](auto x) { return x + x; };
auto bar = [](auto x) { return x - 1; };
auto baz = [](auto x) { std::cout << x << '\n'; return x; };

baz(
bar(
foo(
 42
)));

}
