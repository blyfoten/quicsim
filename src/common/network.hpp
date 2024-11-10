/*
 * Copyright (c) 2024 [Your Name or Organization]
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * Uses:
 * - MsQuic (MIT License)
 * - OpenSSL (OpenSSL/SSLeay License)
 */

#pragma once
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <msquic.h>

class QuicConnection {
public:
    using MessageHandler = std::function<void(const uint8_t*, size_t)>;

    QuicConnection(bool is_server);
    ~QuicConnection();

    bool connect(const std::string& host, uint16_t port);
    bool listen(uint16_t port);
    bool send(const uint8_t* data, size_t len);
    void set_message_handler(MessageHandler handler);
    void poll();

private:
    static const QUIC_API_TABLE* MsQuic;
    static HQUIC Registration;
    static bool InitializeMsQuic();
    
    struct ConnectionContext {
        MessageHandler handler;
        std::vector<uint8_t> recv_buffer;
        bool connected;
    };

    static QUIC_STATUS QUIC_API ConnectionCallback(
        HQUIC Connection,
        void* Context,
        QUIC_CONNECTION_EVENT* Event
    );

    static QUIC_STATUS QUIC_API StreamCallback(
        HQUIC Stream,
        void* Context,
        QUIC_STREAM_EVENT* Event
    );

    HQUIC connection_;
    HQUIC configuration_;
    HQUIC stream_;
    std::unique_ptr<ConnectionContext> context_;
    bool is_server_;
}; 