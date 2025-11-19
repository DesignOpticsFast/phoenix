// TODO(Sprint 4.4): Remove demo-mode local XY Sine once Mac testing is complete.
// This is a temporary path for Capo testing and MUST NOT become permanent.

#include "XYSineDemo.hpp"
#include <cmath>
#include <QDebug>

namespace XYSineDemo {

bool compute(const QMap<QString, QVariant>& params, XYSineResult& outResult)
{
    // Parse parameters with Phoenix-compatible names
    // Defaults match Phoenix FeatureRegistry defaults (same as Bedrock)
    double frequency = 1.0;
    double amplitude = 1.0;
    double phase = 0.0;
    int samples = 1000;
    bool explicitSamplesSet = false;
    
    // Parse parameters from QMap<QString, QVariant>
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
        } else if (key == "samples") {
            // Canonical parameter name (Phoenix standard)
            bool ok;
            int val = value.toInt(&ok);
            if (ok) {
                samples = val;
                explicitSamplesSet = true;
            }
        } else if (key == "n_samples") {
            // Backwards-compatible alias (only used if "samples" not set)
            if (!explicitSamplesSet) {
                bool ok;
                int val = value.toInt(&ok);
                if (ok) {
                    samples = val;
                }
            }
        }
    }
    
    // Validate samples (minimum 2) - matches Bedrock behavior
    if (samples < 2) {
        samples = 2;
    }
    
    // Compute sine wave using Bedrock's exact algorithm
    // t = i / (samples - 1) from 0 to 1
    // x = t * 2π (0..2π domain)
    // y = amplitude * sin(2π * frequency * t + phase)
    outResult.x.clear();
    outResult.y.clear();
    outResult.x.reserve(samples);
    outResult.y.reserve(samples);
    
    for (int i = 0; i < samples; ++i) {
        double t = static_cast<double>(i) / (samples - 1.0);  // 0 to 1
        double x = t * 2.0 * M_PI;  // Scale to 0..2π domain
        double y = amplitude * std::sin(2.0 * M_PI * frequency * t + phase);
        
        outResult.x.push_back(x);
        outResult.y.push_back(y);
    }
    
    return true;
}

} // namespace XYSineDemo

