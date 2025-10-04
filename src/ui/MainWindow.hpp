#pragma once
#include <QMainWindow>
#include <QFutureWatcher>
#include <QString>

class BedrockClient;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent=nullptr);
  ~MainWindow();

private slots:
  void onNewDesign();

private:
  BedrockClient*               client_{nullptr};
  QFutureWatcher<QString>*     watcher_{nullptr};
};
