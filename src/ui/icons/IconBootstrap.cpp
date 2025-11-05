#include "IconBootstrap.h"
#include "PhxLogging.h"
#include <QFontDatabase>
#include <QDebug>

static QString g_ss, g_sr, g_duo, g_br;
static bool g_faAvailable = false;

static bool loadOne(const char* res, QString& family) {
  int id = QFontDatabase::addApplicationFont(res);
  if (id < 0) {
    qCWarning(phxFonts) << "FA font load FAIL:" << res;
    return false;
  }
  auto fams = QFontDatabase::applicationFontFamilies(id);
  if (fams.isEmpty()) {
    qCWarning(phxFonts) << "FA font no families:" << res;
    return false;
  }
  family = fams.first(); // Use actual runtime family
  qCDebug(phxFonts) << "FA font OK" << res << "family:" << family;
  return true;
}

bool IconBootstrap::InitFonts() {
  bool ok = true;
  ok &= loadOne(":/fonts/fa-sharp-solid-900.ttf", g_ss);
  ok &= loadOne(":/fonts/fa-sharp-regular-400.ttf", g_sr);
  ok &= loadOne(":/fonts/fa-duotone-900.ttf", g_duo);
  ok &= loadOne(":/fonts/fa-brands-400.ttf", g_br);
  
  g_faAvailable = ok && !g_ss.isEmpty(); // at minimum, solid required
  if (!g_faAvailable) {
    qCCritical(phxFonts) << "Font Awesome load failed - icons will use fallbacks";
  }
  return g_faAvailable;
}

bool IconBootstrap::faAvailable() {
  return g_faAvailable;
}

const QString& IconBootstrap::sharpSolidFamily()   { return g_ss; }
const QString& IconBootstrap::sharpRegularFamily() { return g_sr; }
const QString& IconBootstrap::duotoneFamily()      { return g_duo; }
const QString& IconBootstrap::brandsFamily()       { return g_br; }