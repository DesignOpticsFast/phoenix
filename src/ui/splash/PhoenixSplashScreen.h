#pragma once

#include <QSplashScreen>
#include <QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class PhoenixSplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
    explicit PhoenixSplashScreen(QWidget *parent = nullptr);
    ~PhoenixSplashScreen() override;

    void setProgress(int value);
    void setMessage(const QString& message);
    qint64 getStartTime() const { return m_startTime; }

protected:
    void drawContents(QPainter *painter) override;

private slots:
    void updateProgress();

private:
    void setupUI();
    void createProgressWidget();
    
    QWidget* m_progressWidget;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QTimer* m_progressTimer;
    
    int m_currentProgress;
    QString m_currentMessage;
    QStringList m_loadingMessages;
    int m_messageIndex;
    
    // Startup timing
    qint64 m_startTime;
};
