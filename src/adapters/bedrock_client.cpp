#include "adapters/bedrock_client.hpp"
#include "bedrock/engine.hpp"

#include <memory>
#include <string>

std::unique_ptr<BedrockClient> BedrockClient::create(bedrock::Engine& engine) {
    return std::unique_ptr<BedrockClient>(new BedrockClient(engine));
}

BedrockClient::BedrockClient(bedrock::Engine& engine) noexcept
    : engine_(engine) {}

BedrockClient::~BedrockClient() noexcept = default;

QString BedrockClient::newDesignTSE_writeSTEP(const QString& outDir) {
    const std::string absPath = engine_.NewDesign_TSE_WriteSTEP(outDir.toStdString());
    return QString::fromStdString(absPath);
}
