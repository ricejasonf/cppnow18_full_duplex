
promise([](auto& resolve, auto x) {
    if (x < 50) {
        resolve(x);
    } else {
        resolve(error{"out of range"});
    }
});
