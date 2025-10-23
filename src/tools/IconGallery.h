#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

class IconGallery : public QWidget {
    Q_OBJECT

public:
    explicit IconGallery(QWidget* parent = nullptr);

private slots:
    void populateIcons();
    void clearIcons();
    void onStyleChanged();
    void onSizeChanged();
    void onThemeToggled();

private:
    QGridLayout* m_iconGrid;
    QComboBox* m_styleCombo;
    QSpinBox* m_sizeSpinBox;
    QCheckBox* m_darkCheckBox;
    QPushButton* m_refreshButton;
};