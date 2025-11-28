#pragma once

#include <QMap>
#include <QVariant>
#include <vector>
#include <QMetaType>

// Result structure for XY Sine computation (Phoenix-only, Phase 2B)
struct XYSineResult {
    std::vector<double> x;
    std::vector<double> y;
};

// Register as Qt meta-type for signal/slot passing
Q_DECLARE_METATYPE(XYSineResult)

// Local XY Sine computation implementation (Phase 2B)
// Provides local compute path without transport dependencies
// Used by LocalExecutor for local-only XY Sine computation
namespace XYSineDemo {
    // Compute XY Sine locally (matches Bedrock's math exactly)
    // Returns true on success, false on failure
    // Validates samples >= 2 (clamps to 2 if less)
    // 
    // Parameters:
    //   - frequency (double, default 1.0)
    //   - amplitude (double, default 1.0)
    //   - phase (double, default 0.0)
    //   - samples (int, default 1000) - also accepts "n_samples" alias
    bool compute(const QMap<QString, QVariant>& params, XYSineResult& outResult);
}

