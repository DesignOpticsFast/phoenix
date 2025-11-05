#pragma once

#include <QSize>

namespace phx {

namespace ui {
    inline constexpr int   kTelemetryIntervalMs    = 1000;
    inline constexpr int   kStatusUpdateMs         = 100;
    inline constexpr QSize kDefaultWindowSize      {1400, 900};
    inline constexpr QSize kMainMinSize            {800, 600};

    inline constexpr int   kMenuIconPx             = 16;
    inline constexpr int   kToolbarIconPx          = 20;
    inline constexpr int   kRibbonIconPx           = 24;

    inline constexpr QSize kPrefsMinSize           {600, 400};
    inline constexpr QSize kPrefsInitSize          {800, 500};

    inline constexpr int   kDockMinWidth           = 200;
    inline constexpr int   kDockWideWidth          = 400;
    inline constexpr int   kPanelMinHeight         = 300;

    inline constexpr int   kUITargetResponseMs     = 50;
}

namespace plot {
    inline constexpr int   kTargetPoints           = 4000;
    inline constexpr bool  kAAWhileIdle            = true;
    inline constexpr bool  kAAWhileInteract        = false;
}

namespace backoff {
    inline constexpr int   kFirstMs                = 250;
    inline constexpr int   kMaxMs                  = 4000;
}

} // namespace phx

