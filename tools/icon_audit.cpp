#include <QCoreApplication>
#include <QResource>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  // Test the actual semantics we have in our .qrc file
  QStringList semantics = {
    "file-plus", "folder", "floppy-disk", "floppy-disk-pen", "sliders", "power-off",
    "lens", "desktop",
    "chart-line", "chart-bar", "save", "open", "info-circle", "circle-question"
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
