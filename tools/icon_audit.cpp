#include <QCoreApplication>
#include <QResource>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  // Minimal canonical semantics we know exist in menus; add others as needed or read from JSON.
  QStringList semantics = {
    "plus", "folder-open", "floppy-disk", "copy", "gear", "xmark",
    "magnifying-glass", "eye",
    "chart-line", "chart-area" // already working, should report FOUND
  };

  int missing = 0;
  for (const auto& s : semantics) {
    const QString svg = QString(":/icons/%1.svg").arg(s);
    bool hasSvg = QFile::exists(svg);
    qInfo().noquote() << (hasSvg ? "[OK]" : "[MISS]") << s << "→" << svg;
    if (!hasSvg) ++missing;
  }
  qInfo() << "Summary: missing" << missing << "of" << semantics.size();
  return missing ? 1 : 0;
}
