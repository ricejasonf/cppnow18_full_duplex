template <typename T>
struct self_deleter {
    T value;

    void done() {
        delete this;
    }
};

int main() {
    auto me = new self_deleter<int>{5};
    me->done();
}
