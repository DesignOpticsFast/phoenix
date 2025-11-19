#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

class EchoTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EchoTestDialog(QWidget* parent = nullptr);
    ~EchoTestDialog() override = default;

private slots:
    void onSendClicked();

private:
    void setupUI();
    void detectBackend();
    void checkLicenseState();
    void appendOutput(const QString& text);
    void clearOutput();
    
    // UI Components
    QLabel* m_backendLabel;
    QLabel* m_licenseStatusLabel;
    QLineEdit* m_messageInput;
    QPushButton* m_sendButton;
    QTextEdit* m_outputText;
    QLabel* m_statusLabel;
    
    QVBoxLayout* m_mainLayout;
    
    // Backend info
    QString m_backendName;
    QString m_envValue;
};

