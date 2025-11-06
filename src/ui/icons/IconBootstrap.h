#pragma once
#include <QString>
#include <QStringList>
#include <QVector>

namespace IconBootstrap {

  struct FontLoadStatus {
    QString path;           // :/fonts/fa-...
    bool exists = false;    // QFile(path).exists()
    qint64 bytes = 0;       // QFileInfo(path).size()
    bool triedPath = false; // attempted addApplicationFont(path)
    bool triedData = false; // attempted addApplicationFontFromData(bytes)
    int id = -1;            // font id returned by addApplicationFont*
    QStringList families;   // applicationFontFamilies(id)
    
    bool ok() const { return id != -1 && !families.isEmpty(); }
  };

  // Face selector: family + styleName (e.g., "Font Awesome 6 Pro" + "Solid")
  struct Face {
    QString family;
    QString style;  // styleName (e.g., "Solid", "Regular")
  };

  bool InitFonts();                        // returns true if all loaded
  bool faAvailable();                      // check if Font Awesome fonts are available
  const QString& sharpSolidFamily();
  const QString& sharpRegularFamily();
  const QString& duotoneFamily();
  const QString& brandsFamily();
  
  // Face selection API
  Face faceForStyle(int iconStyle);  // Get Face for IconStyle enum value
  
  // Diagnostics API
  const QVector<FontLoadStatus>& fontStatuses();
  QStringList expectedFontPaths();
}