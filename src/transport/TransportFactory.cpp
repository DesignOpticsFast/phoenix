#include "TransportFactory.hpp"
#include "LocalSocketChannel.hpp"
#include <memory>

std::unique_ptr<TransportClient> TransportFactory::makeTransportClient(TransportBackend backend)
{
    switch (backend) {
        case TransportBackend::LocalSocket:
        case TransportBackend::Auto:
            // WP1: Both return LocalSocketChannel (no other backends yet)
            return std::make_unique<LocalSocketChannel>();
    }
    
    // Should never reach here, but return LocalSocket as fallback
    return std::make_unique<LocalSocketChannel>();
}

