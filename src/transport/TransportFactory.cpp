#include "TransportFactory.hpp"
#include "GrpcUdsChannel.hpp"
#include "LocalSocketChannel.hpp"
#include <QProcessEnvironment>
#include <QDebug>

std::unique_ptr<TransportClient> makeTransportClient(TransportBackend backend) {
    switch (backend) {
        case TransportBackend::Grpc:
            return std::make_unique<GrpcUdsChannel>();
        case TransportBackend::LocalSocket:
            return std::make_unique<LocalSocketChannel>();
        case TransportBackend::Auto:
            // For now, "auto" means gRPC
            // Fallback logic can be implemented at call site if needed
            return std::make_unique<GrpcUdsChannel>();
    }
    return nullptr;
}

std::unique_ptr<TransportClient> makeTransportClientFromEnv() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString transport = env.value("PHOENIX_TRANSPORT", "auto").toLower();
    
    if (transport == "grpc") {
        qDebug() << "TransportFactory: Using gRPC backend (from PHOENIX_TRANSPORT)";
        return std::make_unique<GrpcUdsChannel>();
    } else if (transport == "localsocket") {
        qDebug() << "TransportFactory: Using LocalSocket backend (from PHOENIX_TRANSPORT)";
        return std::make_unique<LocalSocketChannel>();
    } else {
        // "auto" or unset - default to gRPC for now
        qDebug() << "TransportFactory: Using gRPC backend (auto/default)";
        return std::make_unique<GrpcUdsChannel>();
    }
}

QStringList availableTransportBackends() {
    return QStringList() << "grpc" << "localsocket" << "auto";
}

