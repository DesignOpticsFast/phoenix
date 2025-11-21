#include "GrpcUdsChannel.hpp"

#include <grpcpp/grpcpp.h>
#include <grpcpp/create_channel.h>
#include "bedrock_echo.pb.h"
#include "bedrock_echo.grpc.pb.h"

#include <QDebug>
#include <QByteArray>
#include <memory>
#include <chrono>

using namespace bedrock::echo;

// Helper to delete stub
static void deleteStub(void* ptr) {
    delete static_cast<EchoService::Stub*>(ptr);
}

GrpcUdsChannel::GrpcUdsChannel(const QString& endpoint)
    : endpoint_(endpoint.isEmpty() ? QStringLiteral("localhost:50051") : endpoint)
    , channel_(nullptr)
    , stub_(nullptr, deleteStub)
    , connected_(false)
    , m_lastError(TransportError::NoError)
{
}

GrpcUdsChannel::~GrpcUdsChannel() {
    disconnect();
}

bool GrpcUdsChannel::connect() {
    if (connected_) {
        return true;
    }
    
    try {
        // Create gRPC channel (TCP for now, UDS will be added later)
        // gRPC format: "localhost:50051" for TCP, "unix:/path/to/sock" for UDS
        QByteArray endpointBytes = endpoint_.toUtf8();
        std::shared_ptr<grpc::Channel> grpcChannel = grpc::CreateChannel(
            endpointBytes.constData(),
            grpc::InsecureChannelCredentials()
        );
        
        // Store channel as void* to avoid header dependency
        channel_ = std::shared_ptr<void>(grpcChannel, grpcChannel.get());
        
        // Create stub
        stub_.reset(new EchoService::Stub(grpcChannel));
        
        // Check channel state (non-blocking)
        grpc_connectivity_state state = grpcChannel->GetState(false);
        if (state == GRPC_CHANNEL_SHUTDOWN) {
            qWarning() << "GrpcUdsChannel: Channel is shutdown";
            channel_.reset();
            stub_.reset();
            return false;
        }
        
        connected_ = true;
        qDebug() << "GrpcUdsChannel: Connected to" << endpoint_;
        return true;
    } catch (const std::exception& e) {
        qWarning() << "GrpcUdsChannel: Exception during connect:" << e.what();
        channel_.reset();
        stub_.reset();
        connected_ = false;
        return false;
    }
}

void GrpcUdsChannel::disconnect() {
    if (!connected_) {
        return;
    }
    
    stub_.reset();
    channel_.reset();
    connected_ = false;
    qDebug() << "GrpcUdsChannel: Disconnected";
}

bool GrpcUdsChannel::isConnected() const {
    return connected_ && channel_.get() != nullptr && stub_.get() != nullptr;
}

QString GrpcUdsChannel::backendName() const {
    return QStringLiteral("gRPC (TCP)");
}

QString GrpcUdsChannel::lastErrorString() const {
    return transportErrorString(m_lastError);
}

bool GrpcUdsChannel::echo(const QString& message, QString& reply) {
    if (!isConnected()) {
        qWarning() << "GrpcUdsChannel::echo: Not connected";
        return false;
    }
    
    try {
        EchoRequest request;
        request.set_message(message.toStdString());
        
        EchoReply response;
        grpc::ClientContext context;
        
        // Set a reasonable timeout (5 seconds)
        std::chrono::system_clock::time_point deadline = 
            std::chrono::system_clock::now() + std::chrono::seconds(5);
        context.set_deadline(deadline);
        
        EchoService::Stub* stub = static_cast<EchoService::Stub*>(stub_.get());
        grpc::Status status = stub->Echo(&context, request, &response);
        
        if (!status.ok()) {
            qWarning() << "GrpcUdsChannel::echo: RPC failed -"
                       << "code:" << status.error_code()
                       << "message:" << QString::fromStdString(status.error_message());
            return false;
        }
        
        reply = QString::fromStdString(response.message());
        return true;
    } catch (const std::exception& e) {
        qWarning() << "GrpcUdsChannel::echo: Exception:" << e.what();
        return false;
    }
}

