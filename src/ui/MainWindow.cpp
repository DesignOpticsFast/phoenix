#include "MainWindow.hpp"
#include "adapters/bedrock_client.hpp"
#include "StepViewer.hpp"

#include <QMenuBar>
#include <QAction>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QStatusBar>

using FWatcher = QFutureWatcher<QString>;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  client_ = new BedrockClient;
  watcher_ = new FWatcher(this);

  auto* fileMenu = menuBar()->addMenu("&File");
  auto* newDesign = new QAction("New Design", this);
  fileMenu->addAction(newDesign);
  connect(newDesign, &QAction::triggered, this, &MainWindow::onNewDesign);

  statusBar()->showMessage("Ready");
  resize(900, 600);
}

MainWindow::~MainWindow() {
  delete client_;
}

void MainWindow::onNewDesign() {
  statusBar()->showMessage("Generating STEP...");
  const QString outDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

  auto fut = QtConcurrent::run([this, outDir]{
    return client_->newDesignTSE_writeSTEP(outDir);
  });

  watcher_->setFuture(fut);
  connect(watcher_, &FWatcher::finished, this, [this]{
    const QString path = watcher_->future().result();
    statusBar()->showMessage("STEP created: " + path, 5000);
    auto* v = new StepViewer(this);
    v->setAttribute(Qt::WA_DeleteOnClose);
    v->setPath(path);
    v->setWindowTitle("Phoenix — STEP viewer (placeholder)");
    v->resize(700, 200);
    v->show();
  });
}
