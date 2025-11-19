#include "EchoTestDialog.h"
#include "transport/TransportFactory.hpp"
#include "transport/GrpcUdsChannel.hpp"
#include "transport/LocalSocketChannel.hpp"
#include "app/LicenseManager.h"
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QDebug>
#include <memory>

EchoTestDialog::EchoTestDialog(QWidget* parent)
    : QDialog(parent)
    , m_backendLabel(nullptr)
    , m_licenseStatusLabel(nullptr)
    , m_messageInput(nullptr)
    , m_sendButton(nullptr)
    , m_outputText(nullptr)
    , m_statusLabel(nullptr)
    , m_mainLayout(nullptr)
{
    setWindowTitle(tr("Test Bedrock Connection"));
    setModal(true);
    setMinimumSize(500, 400);
    resize(600, 500);
    
    detectBackend();
    setupUI();
    checkLicenseState();
}

void EchoTestDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Backend info label
    QString backendText = tr("Transport Backend: %1").arg(m_backendName);
    if (!m_envValue.isEmpty()) {
        backendText += tr(" (PHOENIX_TRANSPORT=%1)").arg(m_envValue);
    }
    m_backendLabel = new QLabel(backendText, this);
    m_mainLayout->addWidget(m_backendLabel);
    
    m_mainLayout->addSpacing(10);
    
    // License status label
    m_licenseStatusLabel = new QLabel(this);
    m_mainLayout->addWidget(m_licenseStatusLabel);
    
    m_mainLayout->addSpacing(10);
    
    // Message input
    QLabel* messageLabel = new QLabel(tr("Message:"), this);
    m_mainLayout->addWidget(messageLabel);
    
    m_messageInput = new QLineEdit(this);
    m_messageInput->setText("hello");
    m_mainLayout->addWidget(m_messageInput);
    
    m_mainLayout->addSpacing(10);
    
    // Send button
    m_sendButton = new QPushButton(tr("Send Echo"), this);
    m_sendButton->setDefault(true);
    connect(m_sendButton, &QPushButton::clicked, this, &EchoTestDialog::onSendClicked);
    m_mainLayout->addWidget(m_sendButton);
    
    m_mainLayout->addSpacing(10);
    
    // Output text area
    QLabel* outputLabel = new QLabel(tr("Response:"), this);
    m_mainLayout->addWidget(outputLabel);
    
    m_outputText = new QTextEdit(this);
    m_outputText->setReadOnly(true);
    m_outputText->setMinimumHeight(150);
    m_mainLayout->addWidget(m_outputText);
    
    m_mainLayout->addSpacing(10);
    
    // Status label
    m_statusLabel = new QLabel(tr("Status: Ready"), this);
    m_mainLayout->addWidget(m_statusLabel);
    
    m_mainLayout->addStretch();
    
    // OK button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* okButton = new QPushButton(tr("Close"), this);
    okButton->setDefault(false);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okButton);
    
    m_mainLayout->addLayout(buttonLayout);
}

void EchoTestDialog::detectBackend()
{
    // Get environment value for display
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    m_envValue = env.value("PHOENIX_TRANSPORT", "auto");
    
    // Create a temporary client to detect backend
    auto client = makeTransportClientFromEnv();
    if (client) {
        m_backendName = client->backendName();
    } else {
        m_backendName = tr("Unknown");
    }
}

void EchoTestDialog::checkLicenseState()
{
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    if (state == LicenseManager::LicenseState::NotConfigured) {
        // Licensing not configured - allow feature
        m_licenseStatusLabel->setText(tr("License: Not configured (feature enabled)"));
        m_licenseStatusLabel->setStyleSheet("color: gray;");
        m_sendButton->setEnabled(true);
        m_sendButton->setToolTip(QString());
    } else {
        // Licensing configured - check feature
        bool hasFeature = mgr->hasFeature("feature_echo_test");
        if (hasFeature) {
            m_licenseStatusLabel->setText(tr("License: Valid (feature_echo_test enabled)"));
            m_licenseStatusLabel->setStyleSheet("color: green;");
            m_sendButton->setEnabled(true);
            m_sendButton->setToolTip(QString());
        } else {
            m_licenseStatusLabel->setText(tr("License: feature_echo_test not available"));
            m_licenseStatusLabel->setStyleSheet("color: red;");
            m_sendButton->setEnabled(false);
            m_sendButton->setToolTip(tr("This feature requires the 'feature_echo_test' license."));
        }
    }
}

void EchoTestDialog::onSendClicked()
{
    // Double-check license before sending
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    if (state != LicenseManager::LicenseState::NotConfigured &&
        !mgr->hasFeature("feature_echo_test")) {
        QMessageBox::warning(this, tr("Feature Unavailable"),
            tr("This feature requires the 'feature_echo_test' license.\n\n"
               "Please check your license status via Help → License..."));
        return;
    }
    
    // Clear previous output
    clearOutput();
    m_statusLabel->setText(tr("Status: Sending..."));
    
    // Get message
    QString message = m_messageInput->text();
    if (message.isEmpty()) {
        message = "hello";
    }
    
    // Detect backend type first
    auto tempClient = makeTransportClientFromEnv();
    if (!tempClient) {
        appendOutput(tr("Error: Failed to create transport client"));
        m_statusLabel->setText(tr("Status: Failed - Could not create client"));
        return;
    }
    
    QString backendName = tempClient->backendName();
    bool success = false;
    
    if (backendName.contains("gRPC", Qt::CaseInsensitive)) {
        // gRPC Echo - need to create GrpcUdsChannel specifically
        auto grpcClient = std::make_unique<GrpcUdsChannel>();
        if (!grpcClient->connect()) {
            appendOutput(tr("Error: Failed to connect gRPC client"));
            m_statusLabel->setText(tr("Status: Failed - Connection error"));
            return;
        }
        
        QString reply;
        success = grpcClient->echo(message, reply);
        if (success) {
            appendOutput(tr("Reply: %1").arg(reply));
            m_statusLabel->setText(tr("Status: Success"));
        } else {
            appendOutput(tr("Error: Echo call failed"));
            m_statusLabel->setText(tr("Status: Failed - Echo error"));
        }
        grpcClient->disconnect();
    } else if (backendName.contains("LocalSocket", Qt::CaseInsensitive)) {
        // LocalSocket Capabilities - need to create LocalSocketChannel specifically
        auto lsClient = std::make_unique<LocalSocketChannel>();
        if (!lsClient->connect()) {
            appendOutput(tr("Error: Failed to connect LocalSocket client"));
            m_statusLabel->setText(tr("Status: Failed - Connection error"));
            return;
        }
        
        QStringList features;
        success = lsClient->requestCapabilities(features);
        if (success) {
            appendOutput(tr("Capabilities:"));
            if (features.isEmpty()) {
                appendOutput(tr("  (none)"));
            } else {
                for (const QString& feature : features) {
                    appendOutput(tr("  - %1").arg(feature));
                }
            }
            m_statusLabel->setText(tr("Status: Success"));
        } else {
            appendOutput(tr("Error: Capabilities request failed"));
            m_statusLabel->setText(tr("Status: Failed - Capabilities error"));
        }
        lsClient->disconnect();
    } else {
        appendOutput(tr("Error: Unknown backend type: %1").arg(backendName));
        m_statusLabel->setText(tr("Status: Failed - Unknown backend"));
    }
}

void EchoTestDialog::appendOutput(const QString& text)
{
    if (m_outputText) {
        m_outputText->append(text);
    }
}

void EchoTestDialog::clearOutput()
{
    if (m_outputText) {
        m_outputText->clear();
    }
}

