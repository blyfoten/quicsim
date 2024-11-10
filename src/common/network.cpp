#include "network.hpp"
#include <stdexcept>
#include <iostream>

const QUIC_API_TABLE* QuicConnection::MsQuic = nullptr;
HQUIC QuicConnection::Registration = nullptr;

bool QuicConnection::InitializeMsQuic() {
    if (MsQuic != nullptr) return true;

    QUIC_STATUS status = MsQuicOpen(&MsQuic);
    if (QUIC_FAILED(status)) {
        std::cerr << "MsQuicOpen failed with status: " << status << std::endl;
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
        throw std::runtime_error("Failed to initialize MsQuic");
    }

    context_->connected = false;
    context_->recv_buffer.resize(65535);
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
            conn_context->connected = false;
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
    }
    
    return QUIC_STATUS_SUCCESS;
}

bool QuicConnection::connect(const std::string& host, uint16_t port) {
    QUIC_SETTINGS Settings = {0};
    Settings.IsSet.IdleTimeoutMs = 1;
    Settings.IdleTimeoutMs = 5000;

    QUIC_STATUS status = MsQuic->ConfigurationOpen(
        Registration,
        nullptr, 0,
        &Settings,
        sizeof(Settings),
        nullptr,
        &configuration_);

    if (QUIC_FAILED(status)) return false;

    status = MsQuic->ConnectionOpen(
        Registration,
        ConnectionCallback,
        context_.get(),
        &connection_);

    if (QUIC_FAILED(status)) return false;

    QUIC_ADDR address = {0};
    QuicAddrSetFamily(&address, QUIC_ADDRESS_FAMILY_INET);
    QuicAddrSetPort(&address, port);

    status = MsQuic->ConnectionStart(
        connection_,
        configuration_,
        QUIC_ADDRESS_FAMILY_INET,
        host.c_str(),
        port);

    if (QUIC_FAILED(status)) return false;

    status = MsQuic->StreamOpen(
        connection_,
        QUIC_STREAM_OPEN_FLAG_NONE,
        StreamCallback,
        context_.get(),
        &stream_);

    return !QUIC_FAILED(status);
}

bool QuicConnection::listen(uint16_t port) {
    QUIC_SETTINGS Settings = {0};
    Settings.IsSet.IdleTimeoutMs = 1;
    Settings.IdleTimeoutMs = 5000;

    QUIC_STATUS status = MsQuic->ConfigurationOpen(
        Registration,
        nullptr, 0,
        &Settings,
        sizeof(Settings),
        nullptr,
        &configuration_);

    if (QUIC_FAILED(status)) return false;

    QUIC_ADDR address = {0};
    QuicAddrSetFamily(&address, QUIC_ADDRESS_FAMILY_INET);
    QuicAddrSetPort(&address, port);

    status = MsQuic->ListenerOpen(
        Registration,
        ConnectionCallback,
        context_.get(),
        &connection_);

    if (QUIC_FAILED(status)) return false;

    status = MsQuic->ListenerStart(
        connection_,
        &address);

    return !QUIC_FAILED(status);
}

bool QuicConnection::send(const uint8_t* data, size_t len) {
    if (!stream_ || !context_->connected) return false;

    QUIC_BUFFER buffer;
    buffer.Buffer = const_cast<uint8_t*>(data);
    buffer.Length = static_cast<uint32_t>(len);

    QUIC_STATUS status = MsQuic->StreamSend(
        stream_,
        &buffer,
        1,
        QUIC_SEND_FLAG_NONE,
        nullptr);

    return !QUIC_FAILED(status);
}

void QuicConnection::set_message_handler(MessageHandler handler) {
    context_->handler = std::move(handler);
}

void QuicConnection::poll() {
    // MsQuic is event-driven, no need for explicit polling
} 