#include "IconBootstrap.h"
#include "LogCategories.h"
#include <QFontDatabase>
#include <QDebug>

static QString g_ss, g_sr, g_duo, g_br;

static QString load(const char* res) {
  int id = QFontDatabase::addApplicationFont(res);
  if (id < 0) { qCWarning(phxFonts) << "FA font load FAIL:" << res; return {}; }
  auto fams = QFontDatabase::applicationFontFamilies(id);
  if (fams.isEmpty()) { qCWarning(phxFonts) << "FA font no families:" << res; return {}; }
  qCInfo(phxFonts) << "FA font OK" << res << "families" << fams;
  return fams.first(); // Use actual runtime family
}

bool IconBootstrap::InitFonts() {
  g_ss  = load(":/fonts/fa-sharp-solid-900.ttf");
  g_sr  = load(":/fonts/fa-sharp-regular-400.ttf");
  g_duo = load(":/fonts/fa-duotone-900.ttf");
  g_br  = load(":/fonts/fa-brands-400.ttf");
  return !g_ss.isEmpty(); // at minimum, solid available
}
const QString& IconBootstrap::sharpSolidFamily()   { return g_ss; }
const QString& IconBootstrap::sharpRegularFamily() { return g_sr; }
const QString& IconBootstrap::duotoneFamily()      { return g_duo; }
const QString& IconBootstrap::brandsFamily()       { return g_br; }