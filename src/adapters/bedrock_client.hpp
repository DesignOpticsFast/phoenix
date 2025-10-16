#pragma once
#include <memory>
#include <QString>

namespace bedrock { class IPluginInterface; }

class BedrockClient {
public:
    [[nodiscard]] static std::unique_ptr<BedrockClient> create();
    ~BedrockClient() noexcept;

    BedrockClient(const BedrockClient&) = delete;
    BedrockClient& operator=(const BedrockClient&) = delete;
    BedrockClient(BedrockClient&&) noexcept = default;
    BedrockClient& operator=(BedrockClient&&) = delete;

    // Blocking; call from a worker (QtConcurrent). Returns absolute path to the STEP.
    [[nodiscard]] QString newDesignTSE_writeSTEP(const QString& outDir);

private:
    explicit BedrockClient() noexcept;
    std::unique_ptr<bedrock::IPluginInterface> plugin_;
};
