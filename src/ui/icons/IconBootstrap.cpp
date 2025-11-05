#include "IconBootstrap.h"
#include "PhxLogging.h"
#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QDebug>
#include <algorithm>
#include <mutex>

static QString g_ss, g_sr, g_duo, g_br;
static bool g_faAvailable = false;
static QVector<IconBootstrap::FontLoadStatus> s_status;
static std::once_flag s_infoOnce;

QStringList IconBootstrap::expectedFontPaths() {
  return QStringList{
    ":/fonts/fa-solid-900.ttf",
    ":/fonts/fa-duotone-900.ttf",
    ":/fonts/fa-brands-400.ttf"
    // Note: fa-regular-400.ttf not included yet; all current icons use solid style
  };
}

const QVector<IconBootstrap::FontLoadStatus>& IconBootstrap::fontStatuses() {
  return s_status;
}

bool IconBootstrap::InitFonts() {
  s_status.clear();
  g_faAvailable = false;
  g_ss.clear();
  g_sr.clear();
  g_duo.clear();
  g_br.clear();
  
  QStringList expected = expectedFontPaths();
  
  // Load each font with robust fallback
  for (const QString& path : expected) {
    FontLoadStatus st;
    st.path = path;
    
    // Check existence and size (for diagnostics only, not gating)
    // Try to open the file to check if resource exists
    QFile testFile(path);
    st.exists = testFile.exists();
    if (st.exists) {
      QFileInfo fi(path);
      st.bytes = fi.size();
    }
    
    // Always attempt path-based loading first (don't gate on exists)
    st.triedPath = true;
    st.id = QFontDatabase::addApplicationFont(path);
    
    // Fallback: fromData if path loading failed
    if (st.id == -1) {
      QFile f(path);
      if (f.open(QIODevice::ReadOnly)) {
        st.triedData = true;
        QByteArray data = f.readAll();
        st.bytes = data.size(); // Update bytes from actual loaded data
        st.id = QFontDatabase::addApplicationFontFromData(data);
      }
    }
    
    // Get families if registration succeeded
    if (st.id != -1) {
      st.families = QFontDatabase::applicationFontFamilies(st.id);
      // Update bytes from actual resource if we didn't load via fromData
      if (!st.triedData && st.bytes == 0) {
        QFileInfo fi(path);
        st.bytes = fi.size();
      }
    }
    
    s_status.push_back(st);
    
    // Log per-font status
    if (!st.ok()) {
      // Failed to load - log critical with diagnostic info
      qCCritical(phxFonts) << "FA register failed:" << st.path
                           << "exists=" << st.exists
                           << "bytes=" << st.bytes
                           << "triedPath=" << st.triedPath
                           << "triedData=" << st.triedData;
    } else {
      // Success - log at debug level with details
      QString method = st.triedData ? "fromData" : "byPath";
      qCDebug(phxFonts) << "FA loaded:" << st.path
                        << "method=" << method
                        << "bytes=" << st.bytes
                        << "families=" << st.families.join(", ");
    }
  }
  
  // Determine overall availability (all must be ok)
  g_faAvailable = std::all_of(s_status.begin(), s_status.end(),
                               [](const FontLoadStatus& s) { return s.ok(); });
  
  // Extract family names for backward compatibility
  // Map to Pro fonts: solid (0), duotone (1), brands (2)
  if (s_status.size() >= 3) {
    if (s_status[0].ok() && !s_status[0].families.isEmpty()) {
      g_ss = s_status[0].families.first(); // Pro Solid
    }
    if (s_status[1].ok() && !s_status[1].families.isEmpty()) {
      g_duo = s_status[1].families.first(); // Pro Duotone
    }
    if (s_status[2].ok() && !s_status[2].families.isEmpty()) {
      g_br = s_status[2].families.first(); // Brands
    }
    // g_sr (regular) left empty for now - not used in current manifest
  }
  
  // One summary info log (once per run)
  std::call_once(s_infoOnce, []() {
    int loadedCount = std::count_if(s_status.begin(), s_status.end(),
                                     [](const FontLoadStatus& s) { return s.ok(); });
    qCInfo(phxFonts) << "Font Awesome summary:"
                     << "ok=" << g_faAvailable
                     << "loaded=" << loadedCount
                     << "/" << s_status.size();
  });
  
  return g_faAvailable;
}

bool IconBootstrap::faAvailable() {
  return g_faAvailable;
}

const QString& IconBootstrap::sharpSolidFamily()   { return g_ss; }
const QString& IconBootstrap::sharpRegularFamily() { return g_sr; }
const QString& IconBootstrap::duotoneFamily()      { return g_duo; }
const QString& IconBootstrap::brandsFamily()       { return g_br; }