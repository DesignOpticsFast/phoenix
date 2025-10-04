#pragma once
#include <QMainWindow>
class QFutureWatcherQString;
class BedrockClient;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent=nullptr);
  ~MainWindow();

private slots:
  void onNewDesign();

private:
  BedrockClient* client_{nullptr};
  QFutureWatcherQString* watcher_{nullptr};
};
