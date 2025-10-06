#pragma once

#include <QMainWindow>
#include <QFutureWatcher>
#include <memory>

#include "adapters/bedrock_client.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onExportSTEPFinished();

private:
    std::unique_ptr<Ui::MainWindow> ui_;
    std::unique_ptr<BedrockClient> client_;   // RAII ownership
    QFutureWatcher<QString> watcher_;
};
