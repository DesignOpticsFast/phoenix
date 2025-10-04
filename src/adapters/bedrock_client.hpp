#pragma once
#include <QString>
#include <functional>
namespace bedrock { class Engine; }

class BedrockClient {
public:
  using SomChangedFn = std::function<void(int)>;

  explicit BedrockClient(SomChangedFn cb = nullptr);
  ~BedrockClient();

  // Returns absolute path to STEP file (blocking; call off GUI thread)
  QString newDesignTSE_writeSTEP(const QString& outDir);

private:
  bedrock::Engine* eng_;
};
