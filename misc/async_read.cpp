// Networking TS
template<typename AsyncReadStream,
         typename DynamicBuffer,
         typename ReadHandler>
auto async_read(AsyncReadStream & s,
                DynamicBuffer && buffers,
                ReadHandler && handler);


// Beast Websocket (member function)
template<class DynamicBuffer,
         class ReadHandler>
auto async_read(DynamicBuffer& buffer,
                ReadHandler&& handler);
