
int main() {

auto f = 
    foo(
    bar(
    baz(
     noop
    )));

auto fg = uhhh(f, g);

auto promise_f = do_(
    promise(foo),
    promise(bar),
    promise(baz)
);

auto promise_fg = do_(promise_f, promise_g)

final_promise(promise_fg)(42);

}
