#pragma once

#include <vector>
#include <QVariant>

// Result structure for XY Sine computation
// Shared between transport and demo code
struct XYSineResult {
    std::vector<double> x;
    std::vector<double> y;
};

Q_DECLARE_METATYPE(XYSineResult)

