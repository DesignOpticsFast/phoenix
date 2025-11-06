#include "IconBootstrap.h"
#include "PhxLogging.h"
#include "IconProvider.h"  // For IconStyle enum
#include "app/BuildFlags.h"  // For PHX_DEV_DIAG
#include <QFontDatabase>
#include <QFontMetrics>
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
static IconBootstrap::Face s_faceSolid;
static IconBootstrap::Face s_faceRegular;
static IconBootstrap::Face s_faceDuotone;
static IconBootstrap::Face s_faceBrands;

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
  
  // Detect Font Awesome faces (family + styleName) for macOS compatibility
  // Dump FA families/styles for debugging (guarded by PHX_DEV_DIAG)
#if defined(PHX_DEV_DIAG)
  qCDebug(phxFonts) << "Enumerating Font Awesome families and styles:";
  for (const QString& fam : QFontDatabase::families()) {
    if (fam.startsWith("Font Awesome 6")) {
      QStringList styles = QFontDatabase::styles(fam);
      qCDebug(phxFonts) << "FA family:" << fam << "styles:" << styles;
    }
  }
#endif
  
  // Helper lambda to get styles for a family
  auto stylesOf = [](const QString& fam) -> QStringList {
    return QFontDatabase::styles(fam);
  };
  
  // Detect Pro Solid and Regular faces (macOS often has both under same family)
  const QString proFam = "Font Awesome 6 Pro";
  if (QFontDatabase::hasFamily(proFam)) {
    const QStringList st = stylesOf(proFam);
    if (st.contains("Solid")) {
      s_faceSolid = { proFam, "Solid" };
    }
    if (st.contains("Regular")) {
      s_faceRegular = { proFam, "Regular" };
    }
  }
  
  // Duotone and Brands (usually separate families; style often empty or "Regular")
  const QString duoFam = "Font Awesome 6 Duotone";
  const QString brandsFam = "Font Awesome 6 Brands";
  
  if (QFontDatabase::hasFamily(duoFam)) {
    s_faceDuotone = { duoFam, QString() }; // style usually empty for duotone
  }
  if (QFontDatabase::hasFamily(brandsFam)) {
    s_faceBrands = { brandsFam, QString() }; // style usually empty for brands
  }
  
  // Fallbacks if styleName not found but family exists
  if (!s_faceSolid.family.isEmpty() && s_faceSolid.style.isEmpty()) {
    qCWarning(phxFonts) << "FA Solid styleName missing; falling back to family only";
    // Keep family-only fallback
  }
  if (!s_faceRegular.family.isEmpty() && s_faceRegular.style.isEmpty()) {
    qCWarning(phxFonts) << "FA Regular styleName missing; falling back to family only";
    // Keep family-only fallback
  }
  
  // One summary log (once per run, debug level for production)
  std::call_once(s_infoOnce, []() {
    int loadedCount = std::count_if(s_status.begin(), s_status.end(),
                                     [](const FontLoadStatus& s) { return s.ok(); });
    qCDebug(phxFonts) << "Font Awesome summary:"
                     << "ok=" << g_faAvailable
                     << "loaded=" << loadedCount
                     << "/" << s_status.size();
  });
  
  // One-time glyph availability probes for common icons (guarded by PHX_DEV_DIAG)
  // This helps verify that manifest codepoints exist in the loaded FA fonts
#if defined(PHX_DEV_DIAG)
  if (g_faAvailable) {
    // Forward declaration helper for faceHasGlyph (defined in IconProvider.cpp)
    // We'll call it via a lambda that uses QFontDatabase directly
    auto faceHasGlyph = [](const QString& fam, const QString& style, char32_t cp) -> bool {
      QFont probe = QFontDatabase::font(fam, style, 12);
      if (probe.family().isEmpty()) {
        probe = QFont(fam);
        if (!style.isEmpty()) {
          probe.setStyleName(style);
        }
      }
      probe.setStyleStrategy(QFont::NoFontMerging);
      // Use QFontMetrics to check if the glyph exists in the font
      QFontMetrics fm(probe);
      QChar ch(cp);
      return fm.inFont(ch);
    };
    
    struct Probe {
      const char* name;
      uint32_t cp;
      IconStyle st;
    };
    const Probe probes[] = {
      {"save", 0xF0C7, IconStyle::SharpSolid},
      {"folder-open", 0xF07C, IconStyle::SharpSolid},
      {"file-plus", 0xF0FE, IconStyle::SharpSolid},
      {"info", 0xF129, IconStyle::SharpSolid},
      {"plus", 0xF067, IconStyle::SharpSolid},
    };
    
    qCDebug(phxFonts) << "Probing glyph availability for common icons:";
    for (const auto& pr : probes) {
      auto face = IconBootstrap::faceForStyle(static_cast<int>(pr.st));
      const bool ok = faceHasGlyph(face.family, face.style, static_cast<char32_t>(pr.cp));
      qCDebug(phxFonts) << "probe" << pr.name
                       << "cp" << QString::number(pr.cp, 16)
                       << "fam" << face.family
                       << "style" << face.style
                       << "ok" << ok;
    }
  }
#endif
  
  return g_faAvailable;
}

bool IconBootstrap::faAvailable() {
  return g_faAvailable;
}

const QString& IconBootstrap::sharpSolidFamily()   { return g_ss; }
const QString& IconBootstrap::sharpRegularFamily() { return g_sr; }
const QString& IconBootstrap::duotoneFamily()      { return g_duo; }
const QString& IconBootstrap::brandsFamily()       { return g_br; }

IconBootstrap::Face IconBootstrap::faceForStyle(int iconStyle) {
  // Map IconStyle enum to Face
  // Note: IconStyle::SharpSolid = 0, SharpRegular = 1, Duotone = 2, Brands = 3, ClassicSolid = 4
  switch (static_cast<IconStyle>(iconStyle)) {
    case IconStyle::SharpSolid:
    case IconStyle::ClassicSolid:
      // Both Solid and ClassicSolid use Pro Solid face
      if (s_faceSolid.family.isEmpty()) {
        return Face{ "Font Awesome 6 Pro", QString() };
      }
      return s_faceSolid;
    
    case IconStyle::SharpRegular:
      if (s_faceRegular.family.isEmpty()) {
        return Face{ "Font Awesome 6 Pro", QString() };
      }
      return s_faceRegular;
    
    case IconStyle::Duotone:
      if (s_faceDuotone.family.isEmpty()) {
        return Face{ "Font Awesome 6 Duotone", QString() };
      }
      return s_faceDuotone;
    
    case IconStyle::Brands:
      if (s_faceBrands.family.isEmpty()) {
        return Face{ "Font Awesome 6 Brands", QString() };
      }
      return s_faceBrands;
    
    default:
      return Face{ "Font Awesome 6 Pro", QString() };
  }
}