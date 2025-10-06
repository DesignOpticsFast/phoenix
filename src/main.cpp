#include "ui/MainWindow.hpp"
#include <QApplication>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  MainWindow w;
  w.setWindowTitle("Phoenix (Sprint 1)");
  w.show();
  return app.exec();
}
