#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QtConcurrent/QtConcurrent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include "adapters/bedrock_client.hpp"
#include "bedrock/engine.hpp"   // ensures complete type for bedrock::Engine

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui_(std::make_unique<Ui::MainWindow>())
{
    ui_->setupUi(this);

    // Create/obtain an Engine and a BedrockClient via factory
    static bedrock::Engine engine;
    client_ = BedrockClient::create(engine);

    // Hook up STEP export button
    if (ui_->btnExportSTEP) {
        connect(ui_->btnExportSTEP, &QPushButton::clicked, this, [this]() {
            const QString outDir =
                QFileDialog::getExistingDirectory(this, "Select Output Directory");
            if (outDir.isEmpty())
                return;

            // BedrockClient is not a QObject → capture a raw non-owning pointer
            BedrockClient* clientPtr = client_.get();

            auto future = QtConcurrent::run([clientPtr, outDir]() -> QString {
                try {
                    // Real exporter call — blocking off the GUI thread
                    return clientPtr->newDesignTSE_writeSTEP(outDir);
                } catch (const std::exception& ex) {
                    return QString("Error: ") + ex.what();
                }
            });

            watcher_.setFuture(future);
            connect(&watcher_, &QFutureWatcher<QString>::finished,
                    this, &MainWindow::onExportSTEPFinished);
        });
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::onExportSTEPFinished()
{
    const QString result = watcher_.future().result();
    if (result.startsWith("Error:"))
        QMessageBox::critical(this, "STEP Export Failed", result);
    else
        QMessageBox::information(this, "STEP Export Complete",
                                 "STEP file saved:\n" + result);
}
