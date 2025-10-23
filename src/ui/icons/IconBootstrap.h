#pragma once
#include <QString>

namespace IconBootstrap {
  bool InitFonts();                        // returns true if all loaded
  const QString& sharpSolidFamily();
  const QString& sharpRegularFamily();
  const QString& duotoneFamily();
  const QString& brandsFamily();
}