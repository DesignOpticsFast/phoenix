#include "adapters/bedrock_client.hpp"
#include <bedrock/engine.hpp>

BedrockClient::BedrockClient(SomChangedFn cb)
  : eng_(new bedrock::Engine(std::move(cb))) {}

BedrockClient::~BedrockClient() { delete eng_; }

QString BedrockClient::newDesignTSE_writeSTEP(const QString& outDir) {
  return QString::fromStdString(eng_->NewDesign_TSE_WriteSTEP(outDir.toStdString()));
}
