#include "PhoenixSplashScreen.h"
#include "version.h"
#include <QApplication>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QScreen>
#include <QDateTime>
#include <QDebug>

PhoenixSplashScreen::PhoenixSplashScreen(QWidget *parent)
    : QSplashScreen(QPixmap(), Qt::SplashScreen | Qt::FramelessWindowHint)
    , m_progressWidget(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_progressTimer(nullptr)
    , m_currentProgress(0)
    , m_messageIndex(0)
    , m_subtitleText()  // Will be set in initializeMessages() after translators are active
    , m_startTime(QDateTime::currentMSecsSinceEpoch())
{
    setupUI();
    createProgressWidget();
    
    // Start progress animation
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &PhoenixSplashScreen::updateProgress);
    m_progressTimer->start(200); // Update every 200ms
    
    // Note: m_loadingMessages will be populated by initializeMessages() after translators are installed
}

PhoenixSplashScreen::~PhoenixSplashScreen()
{
    if (m_progressTimer) {
        m_progressTimer->stop();
    }
}

void PhoenixSplashScreen::initializeMessages()
{
    m_loadingMessages.clear();
    m_loadingMessages << tr("Initializing Phoenix...")
                     << tr("Loading Font Awesome icons...")
                     << tr("Setting up theme system...")
                     << tr("Preparing user interface...")
                     << tr("Loading preferences...")
                     << tr("Almost ready...");
    
    // Cache translated subtitle for use in drawContents()
    m_subtitleText = tr("Optical Design Studio");
    
    // Set initial message if we have messages
    if (!m_loadingMessages.isEmpty()) {
        setMessage(m_loadingMessages.first());
    }
}

void PhoenixSplashScreen::setupUI()
{
    // Set splash screen size
    setFixedSize(500, 300);
    
    // Center on screen
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Set window flags
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void PhoenixSplashScreen::createProgressWidget()
{
    m_progressWidget = new QWidget(this);
    m_progressWidget->setFixedSize(400, 80);
    m_progressWidget->move(50, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(m_progressWidget);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(5);
    
    // Progress bar
    m_progressBar = new QProgressBar(m_progressWidget);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(8);
    
    // Status label (text will be set in initializeMessages() after translators are active)
    m_statusLabel = new QLabel(QString(), m_progressWidget);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #666666; font-size: 11px;");
    
    layout->addWidget(m_progressBar);
    layout->addWidget(m_statusLabel);
}

void PhoenixSplashScreen::setProgress(int value)
{
    m_currentProgress = qBound(0, value, 100);
    if (m_progressBar) {
        m_progressBar->setValue(m_currentProgress);
    }
}

void PhoenixSplashScreen::setMessage(const QString& message)
{
    m_currentMessage = message;
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
    update(); // Trigger repaint
}

void PhoenixSplashScreen::updateProgress()
{
    // Simulate loading progress
    if (m_currentProgress < 100) {
        m_currentProgress += 2;
        setProgress(m_currentProgress);
        
        // Update message based on progress (only if messages are initialized)
        if (!m_loadingMessages.isEmpty()) {
            int messageIndex = (m_currentProgress * m_loadingMessages.size()) / 100;
            if (messageIndex < m_loadingMessages.size()) {
                setMessage(m_loadingMessages[messageIndex]);
            }
        }
    } else {
        // Progress complete
        m_progressTimer->stop();
        setMessage(tr("Ready!"));
    }
}

void PhoenixSplashScreen::drawContents(QPainter *painter)
{
    // Set up painter
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw background gradient
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(240, 240, 240));
    gradient.setColorAt(1, QColor(220, 220, 220));
    
    painter->fillRect(rect(), gradient);
    
    // Draw border
    painter->setPen(QPen(QColor(200, 200, 200), 1));
    painter->drawRect(rect().adjusted(0, 0, -1, -1));
    
    // Draw title
    QFont titleFont("Arial", 24, QFont::Bold);
    painter->setFont(titleFont);
    painter->setPen(QColor(50, 50, 50));
    
    QFontMetrics titleMetrics(titleFont);
    QString title = "Phoenix";
    QRect titleRect = titleMetrics.boundingRect(title);
    int titleX = (width() - titleRect.width()) / 2;
    int titleY = 60;
    
    painter->drawText(titleX, titleY, title);
    
    // Draw subtitle
    QFont subtitleFont("Arial", 12);
    painter->setFont(subtitleFont);
    painter->setPen(QColor(100, 100, 100));
    
    QFontMetrics subtitleMetrics(subtitleFont);
    // Use cached subtitle text (set in initializeMessages() after translators are active)
    QString subtitle = m_subtitleText.isEmpty() ? QStringLiteral("Optical Design Studio") : m_subtitleText;
    QRect subtitleRect = subtitleMetrics.boundingRect(subtitle);
    int subtitleX = (width() - subtitleRect.width()) / 2;
    int subtitleY = titleY + 35;
    
    painter->drawText(subtitleX, subtitleY, subtitle);
    
    // Draw version
    QFont versionFont("Arial", 10);
    painter->setFont(versionFont);
    painter->setPen(QColor(150, 150, 150));
    
    QString version = QStringLiteral("Version %1").arg(QStringLiteral(PHOENIX_VERSION));
    QFontMetrics versionMetrics(versionFont);
    QRect versionRect = versionMetrics.boundingRect(version);
    int versionX = (width() - versionRect.width()) / 2;
    int versionY = subtitleY + 25;
    
    painter->drawText(versionX, versionY, version);
}
