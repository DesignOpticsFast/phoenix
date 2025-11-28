#include "RemoteExecutor.hpp"

// Include transport client (only when PHX_WITH_TRANSPORT_DEPS=ON)
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "transport/TransportFactory.hpp"
#include "transport/TransportClient.hpp"
#include "transport/LocalSocketChannel.hpp"
// Proto header is in generated directory, included via CMake include paths
#include "palantir/capabilities.pb.h"
#include "palantir/xysine.pb.h"
#include "analysis/demo/XYSineDemo.hpp"
#endif

#include <atomic>
#include <QDebug>

RemoteExecutor::RemoteExecutor()
    : m_cancelled(false)
#ifdef PHX_WITH_TRANSPORT_DEPS
    , m_transport(TransportFactory::makeTransportClient(TransportBackend::Auto))
#else
    , m_transport(nullptr)
#endif
{
}

RemoteExecutor::~RemoteExecutor() = default;

void RemoteExecutor::execute(
    const QString& featureId,
    const QMap<QString, QVariant>& params,
    ProgressCallback onProgress,
    ResultCallback onResult,
    ErrorCallback onError)
{
    // Reset cancellation flag
    m_cancelled.store(false);

#ifdef PHX_WITH_TRANSPORT_DEPS
    if (!m_transport) {
        if (onError) {
            onError(QString("Transport client not available"));
        }
        return;
    }
    
    // Connect to remote service
    if (!m_transport->connect()) {
        QString errorMsg = "Unable to connect to remote analysis service";
        if (onError) {
            onError(errorMsg);
        }
        return;
    }
    
    // Check for cancellation
    if (m_cancelled.load()) {
        if (onError) {
            onError(QString("Computation cancelled"));
        }
        return;
    }
    
    // Fetch capabilities from remote server
    QString errorMsg;
    auto capabilities = m_transport->getCapabilities(&errorMsg);
    
    if (!capabilities.has_value()) {
        if (onError) {
            onError(errorMsg.isEmpty() ? QString("Failed to fetch capabilities") : errorMsg);
        }
        return;
    }
    
    // Log capabilities for debugging
    qDebug() << "Capabilities fetched: server_version=" 
             << QString::fromStdString(capabilities->capabilities().server_version())
             << "features=" << capabilities->capabilities().supported_features_size();
    
    // Check if requested feature is supported
    QString requestedFeature = featureId;
    bool featureSupported = false;
    for (int i = 0; i < capabilities->capabilities().supported_features_size(); ++i) {
        if (QString::fromStdString(capabilities->capabilities().supported_features(i)) == requestedFeature) {
            featureSupported = true;
            break;
        }
    }
    
    if (!featureSupported) {
        if (onError) {
            onError(QString("Feature '%1' not supported by server").arg(requestedFeature));
        }
        return;
    }
    
    // Execute remote computation based on featureId
    if (requestedFeature == "xy_sine") {
        // Check for cancellation
        if (m_cancelled.load()) {
            if (onError) {
                onError(QString("Computation cancelled"));
            }
            return;
        }
        
        // Report progress start
        if (onProgress) {
            onProgress(0.0);
        }
        
        // Build XYSineRequest from params
        palantir::XYSineRequest request;
        
        // Parse parameters with defaults matching XYSineDemo
        double frequency = 1.0;
        double amplitude = 1.0;
        double phase = 0.0;
        int samples = 1000;
        
        for (auto it = params.begin(); it != params.end(); ++it) {
            QString key = it.key();
            QVariant value = it.value();
            
            if (key == "frequency") {
                bool ok;
                double val = value.toDouble(&ok);
                if (ok) {
                    frequency = val;
                }
            } else if (key == "amplitude") {
                bool ok;
                double val = value.toDouble(&ok);
                if (ok) {
                    amplitude = val;
                }
            } else if (key == "phase") {
                bool ok;
                double val = value.toDouble(&ok);
                if (ok) {
                    phase = val;
                }
            } else if (key == "samples" || key == "n_samples") {
                bool ok;
                int val = value.toInt(&ok);
                if (ok && val > 0) {
                    samples = val;
                }
            }
        }
        
        // Set request fields
        request.set_frequency(frequency);
        request.set_amplitude(amplitude);
        request.set_phase(phase);
        request.set_samples(samples);
        
        // Send XY Sine request via transport
        // Note: We need to cast to LocalSocketChannel to access sendXYSineRequest
        // This is acceptable since TransportFactory currently only returns LocalSocketChannel
        auto* localChannel = dynamic_cast<LocalSocketChannel*>(m_transport.get());
        if (!localChannel) {
            if (onError) {
                onError(QString("Transport client does not support XY Sine RPC"));
            }
            return;
        }
        
        QString rpcError;
        auto response = localChannel->sendXYSineRequest(request, &rpcError);
        
        // Check for cancellation after RPC
        if (m_cancelled.load()) {
            if (onError) {
                onError(QString("Computation cancelled"));
            }
            return;
        }
        
        if (!response.has_value()) {
            if (onError) {
                onError(rpcError.isEmpty() ? QString("XY Sine RPC failed") : rpcError);
            }
            return;
        }
        
        // Convert XYSineResponse to XYSineResult
        XYSineResult result;
        result.x.reserve(response->x_size());
        result.y.reserve(response->y_size());
        
        for (int i = 0; i < response->x_size(); ++i) {
            result.x.push_back(response->x(i));
        }
        for (int i = 0; i < response->y_size(); ++i) {
            result.y.push_back(response->y(i));
        }
        
        // Validate result
        if (result.x.size() != result.y.size()) {
            if (onError) {
                onError(QString("Invalid response: x and y arrays have different sizes"));
            }
            return;
        }
        
        // Report progress complete
        if (onProgress) {
            onProgress(1.0);
        }
        
        // Emit success with result
        if (onResult) {
            onResult(result);
        }
        return;
    }
    
    // Unknown feature
    if (onError) {
        onError(QString("Remote execution for '%1' not yet implemented").arg(requestedFeature));
    }
#else
    // Transport deps not available
    if (onError) {
        onError(QString("Transport dependencies not enabled (PHX_WITH_TRANSPORT_DEPS=OFF)"));
    }
#endif
}

void RemoteExecutor::cancel()
{
    // WP1: Simple flag-based cancellation
    // Future: Cancel ongoing Bedrock requests
    m_cancelled.store(true);
}

