#pragma once
#include <memory>
#include <QString>

namespace bedrock { class Engine; }

class BedrockClient {
public:
    [[nodiscard]] static std::unique_ptr<BedrockClient> create(bedrock::Engine& engine);
    ~BedrockClient() noexcept;

    BedrockClient(const BedrockClient&) = delete;
    BedrockClient& operator=(const BedrockClient&) = delete;
    BedrockClient(BedrockClient&&) noexcept = default;
    BedrockClient& operator=(BedrockClient&&) = delete; // cannot reseat a reference

    // Blocking; call from a worker (QtConcurrent). Returns absolute path to the STEP.
    [[nodiscard]] QString newDesignTSE_writeSTEP(const QString& outDir);

private:
    explicit BedrockClient(bedrock::Engine& engine) noexcept;
    bedrock::Engine& engine_; // non-null by type
};
