#pragma once

#include <QMap>
#include <QVariant>
#include <vector>

// Result structure for XY Sine computation (matches LocalSocketChannel)
struct XYSineResult {
    std::vector<double> x;
    std::vector<double> y;
};

// TODO(Sprint 4.4): Remove demo-mode local XY Sine once Mac testing is complete.
// This is a temporary path for Capo testing and MUST NOT become permanent.
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

