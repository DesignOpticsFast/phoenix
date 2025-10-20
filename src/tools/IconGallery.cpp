#include "IconGallery.h"
#include "../ui/icons/IconProvider.h"
#include "../ui/icons/IconBootstrap.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QDebug>

IconGallery::IconGallery(QWidget* parent)
    : QWidget(parent)
    , m_iconGrid(new QGridLayout())
    , m_styleCombo(new QComboBox())
    , m_sizeSpinBox(new QSpinBox())
    , m_darkCheckBox(new QCheckBox("Dark Mode"))
    , m_refreshButton(new QPushButton("Refresh"))
{
    setWindowTitle("Phoenix Icon Gallery");
    setMinimumSize(800, 600);
    
    // Setup controls
    m_styleCombo->addItems({"SharpSolid", "SharpRegular", "Duotone", "Brands", "ClassicSolid"});
    m_sizeSpinBox->setRange(16, 128);
    m_sizeSpinBox->setValue(24);
    
    // Layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Controls
    QHBoxLayout* controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel("Style:"));
    controlsLayout->addWidget(m_styleCombo);
    controlsLayout->addWidget(new QLabel("Size:"));
    controlsLayout->addWidget(m_sizeSpinBox);
    controlsLayout->addWidget(m_darkCheckBox);
    controlsLayout->addWidget(m_refreshButton);
    controlsLayout->addStretch();
    
    // Scroll area for icons
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollWidget = new QWidget();
    scrollWidget->setLayout(m_iconGrid);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    
    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(scrollArea);
    
    // Connect signals
    connect(m_refreshButton, &QPushButton::clicked, this, &IconGallery::populateIcons);
    connect(m_styleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IconGallery::onStyleChanged);
    connect(m_sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &IconGallery::onSizeChanged);
    connect(m_darkCheckBox, &QCheckBox::toggled, this, &IconGallery::onThemeToggled);
    
    // Initial population
    populateIcons();
}

void IconGallery::populateIcons() {
    clearIcons();
    
    // Add font family info
    QLabel* fontInfoLabel = new QLabel("Font families: " + IconBootstrap::loadedFontFamilies().join(", "));
    m_iconGrid->addWidget(fontInfoLabel, 0, 0, 1, 6);
    
    // Sample icons to display
    QStringList iconNames = {
        "save", "open", "chart-line", "chart-scatter", "wave-sine",
        "cube", "ruler-combined", "brain", "circle-question"
    };
    
    int row = 1;
    int col = 0;
    const int maxCols = 6;
    
    for (const QString& iconName : iconNames) {
        QLabel* iconLabel = new QLabel();
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setMinimumSize(100, 100);
        iconLabel->setStyleSheet("border: 1px solid #ccc; margin: 2px;");
        
        // Get icon
        IconStyle style = static_cast<IconStyle>(m_styleCombo->currentIndex());
        int size = m_sizeSpinBox->value();
        bool dark = m_darkCheckBox->isChecked();
        
        QIcon icon = IconProvider::icon(iconName, style, size, dark);
        iconLabel->setPixmap(icon.pixmap(size, size));
        
        // Add name label
        QLabel* nameLabel = new QLabel(iconName);
        nameLabel->setAlignment(Qt::AlignCenter);
        
        QVBoxLayout* itemLayout = new QVBoxLayout();
        itemLayout->addWidget(iconLabel);
        itemLayout->addWidget(nameLabel);
        
        QWidget* itemWidget = new QWidget();
        itemWidget->setLayout(itemLayout);
        
        m_iconGrid->addWidget(itemWidget, row, col);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void IconGallery::clearIcons() {
    QLayoutItem* item;
    while ((item = m_iconGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void IconGallery::onStyleChanged() {
    populateIcons();
}

void IconGallery::onSizeChanged() {
    populateIcons();
}

void IconGallery::onThemeToggled() {
    populateIcons();
}