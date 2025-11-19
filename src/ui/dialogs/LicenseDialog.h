#pragma once

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class LicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LicenseDialog(QWidget* parent = nullptr);
    ~LicenseDialog() override = default;

private slots:
    void onRefreshClicked();
    void updateLicenseDisplay();

private:
    void setupUI();
    QString stateToString() const;
    QColor stateColor() const;
    
    // UI Components
    QLabel* m_statusLabel;
    QLabel* m_subjectLabel;
    QLabel* m_issuedLabel;
    QLabel* m_expiresLabel;
    QListWidget* m_featuresList;
    QPushButton* m_refreshButton;
    QPushButton* m_okButton;
    
    QVBoxLayout* m_mainLayout;
};

