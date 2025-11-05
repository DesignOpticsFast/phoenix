#pragma once
#include <QString>

namespace IconBootstrap {
  bool InitFonts();                        // returns true if all loaded
  bool faAvailable();                      // check if Font Awesome fonts are available
  const QString& sharpSolidFamily();
  const QString& sharpRegularFamily();
  const QString& duotoneFamily();
  const QString& brandsFamily();
}