auto endpoint_compose(auto T, auto U) {
    return endoint(
        init          = do_(T.init, U.init]),
        read_message  = do_(T.read_message,  U.read_message),
        write_message = do_(U.write_message, T.write_message));
}

