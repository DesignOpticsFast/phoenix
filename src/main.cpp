#include "app/MainWindow.hpp"
#include <QApplication>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  
  // High DPI scaling is enabled by default in Qt 6
  
  MainWindow w;
  w.show();
  return app.exec();
}
