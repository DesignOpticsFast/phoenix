#include "adapters/bedrock_client.hpp"
#include "bedrock/plugin_interface.hpp"

#include <memory>
#include <string>

std::unique_ptr<BedrockClient> BedrockClient::create() {
    return std::unique_ptr<BedrockClient>(new BedrockClient());
}

BedrockClient::BedrockClient() noexcept
    : plugin_(bedrock::createPluginInterface()) {}

BedrockClient::~BedrockClient() noexcept = default;

QString BedrockClient::newDesignTSE_writeSTEP(const QString& outDir) {
    const std::string absPath = plugin_->NewDesign_TSE_WriteSTEP(outDir.toStdString());
    return QString::fromStdString(absPath);
}
