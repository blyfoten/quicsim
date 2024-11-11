#include "network.hpp"
#include <stdexcept>
#include <iostream>

const QUIC_API_TABLE* QuicConnection::MsQuic = nullptr;
HQUIC QuicConnection::Registration = nullptr;

bool QuicConnection::InitializeMsQuic() {
    if (MsQuic != nullptr) return true;

    QUIC_STATUS status = MsQuicOpen2(&MsQuic);
    if (QUIC_FAILED(status)) {
        std::cerr << "MsQuicOpen2 failed with status: " << status << std::endl;
        return false;
    }

    QUIC_REGISTRATION_CONFIG RegConfig = {
        "simulation_app",
        QUIC_EXECUTION_PROFILE_LOW_LATENCY
    };

    status = MsQuic->RegistrationOpen(&RegConfig, &Registration);
    if (QUIC_FAILED(status)) {
        std::cerr << "RegistrationOpen failed with status: " << status << std::endl;
        MsQuicClose(MsQuic);
        MsQuic = nullptr;
        return false;
    }

    return true;
}

QuicConnection::QuicConnection(bool is_server) 
    : connection_(nullptr)
    , configuration_(nullptr)
    , stream_(nullptr)
    , context_(std::make_unique<ConnectionContext>())
    , is_server_(is_server) {
    
    if (!InitializeMsQuic()) {
        throw std::runtime_error("Failed to initialize MSQUIC");
    }

    QUIC_SETTINGS Settings = {0};
    Settings.IdleTimeoutMs = 5000;
    Settings.IsSet.IdleTimeoutMs = TRUE;

    QUIC_STATUS status = MsQuic->ConfigurationOpen(
        Registration,
        nullptr, 0,  // No ALPN needed for our use case
        &Settings,
        sizeof(Settings),
        nullptr,
        &configuration_
    );

    if (QUIC_FAILED(status)) {
        throw std::runtime_error("Failed to open configuration");
    }
}

QuicConnection::~QuicConnection() {
    if (stream_) MsQuic->StreamClose(stream_);
    if (connection_) MsQuic->ConnectionClose(connection_);
    if (configuration_) MsQuic->ConfigurationClose(configuration_);
}

QUIC_STATUS QuicConnection::ConnectionCallback(
    HQUIC Connection,
    void* Context,
    QUIC_CONNECTION_EVENT* Event
) {
    auto conn_context = static_cast<ConnectionContext*>(Context);
    
    switch (Event->Type) {
        case QUIC_CONNECTION_EVENT_CONNECTED:
            conn_context->connected = true;
            break;
            
        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
            // Handle shutdown
            break;
            
        case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
            // Handle complete shutdown
            break;
    }
    
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicConnection::StreamCallback(
    HQUIC Stream,
    void* Context,
    QUIC_STREAM_EVENT* Event
) {
    auto conn_context = static_cast<ConnectionContext*>(Context);
    
    switch (Event->Type) {
        case QUIC_STREAM_EVENT_RECEIVE:
            if (conn_context->handler) {
                conn_context->handler(
                    Event->RECEIVE.Buffers[0].Buffer,
                    Event->RECEIVE.Buffers[0].Length
                );
            }
            break;
            
        case QUIC_STREAM_EVENT_SEND_COMPLETE:
            // Handle send complete
            break;
    }
    
    return QUIC_STATUS_SUCCESS;
}

bool QuicConnection::connect(const std::string& host, uint16_t port) {
    if (is_server_) return false;

    QUIC_STATUS status = MsQuic->ConnectionOpen(
        Registration,
        ConnectionCallback,
        context_.get(),
        &connection_
    );

    if (QUIC_FAILED(status)) {
        std::cerr << "ConnectionOpen failed with status: " << status << std::endl;
        return false;
    }

    status = MsQuic->ConnectionStart(
        connection_,
        configuration_,
        QUIC_ADDRESS_FAMILY_UNSPEC,
        host.c_str(),
        port
    );

    if (QUIC_FAILED(status)) {
        std::cerr << "ConnectionStart failed with status: " << status << std::endl;
        return false;
    }

    return true;
}

bool QuicConnection::listen(uint16_t port) {
    if (!is_server_) return false;

    QUIC_ADDR addr = {0};
    QuicAddrSetPort(&addr, port);

    auto ListenerCallback = [](
        HQUIC Listener,
        void* Context,
        QUIC_LISTENER_EVENT* Event
    ) -> QUIC_STATUS {
        auto conn_context = static_cast<ConnectionContext*>(Context);
        
        if (Event->Type == QUIC_LISTENER_EVENT_NEW_CONNECTION) {
            return ConnectionCallback(
                Event->NEW_CONNECTION.Connection,
                Context,
                nullptr
            );
        }
        return QUIC_STATUS_SUCCESS;
    };

    QUIC_STATUS status = MsQuic->ListenerOpen(
        Registration,
        ListenerCallback,
        context_.get(),
        &connection_
    );

    if (QUIC_FAILED(status)) {
        std::cerr << "ListenerOpen failed with status: " << status << std::endl;
        return false;
    }

    QUIC_BUFFER alpn = { sizeof("simulation") - 1, (uint8_t*)"simulation" };
    status = MsQuic->ListenerStart(
        connection_,
        &alpn,
        1,
        &addr
    );

    if (QUIC_FAILED(status)) {
        std::cerr << "ListenerStart failed with status: " << status << std::endl;
        return false;
    }

    return true;
}

bool QuicConnection::send(const uint8_t* data, size_t len) {
    if (!connection_ || !context_->connected) return false;

    if (!stream_) {
        QUIC_STATUS status = MsQuic->StreamOpen(
            connection_,
            QUIC_STREAM_OPEN_FLAG_NONE,
            StreamCallback,
            context_.get(),
            &stream_
        );

        if (QUIC_FAILED(status)) {
            std::cerr << "StreamOpen failed with status: " << status << std::endl;
            return false;
        }

        status = MsQuic->StreamStart(stream_, QUIC_STREAM_START_FLAG_NONE);
        if (QUIC_FAILED(status)) {
            std::cerr << "StreamStart failed with status: " << status << std::endl;
            return false;
        }
    }

    QUIC_BUFFER buffer = {
        static_cast<uint32_t>(len),
        const_cast<uint8_t*>(data)
    };

    QUIC_STATUS status = MsQuic->StreamSend(
        stream_,
        &buffer,
        1,
        QUIC_SEND_FLAG_NONE,
        nullptr
    );

    if (QUIC_FAILED(status)) {
        std::cerr << "StreamSend failed with status: " << status << std::endl;
        return false;
    }

    return true;
}

void QuicConnection::set_message_handler(MessageHandler handler) {
    context_->handler = std::move(handler);
}

void QuicConnection::poll() {
    // MSQUIC is event-driven, no need for explicit polling
} 