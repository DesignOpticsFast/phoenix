#include "EnvironmentPrefs.hpp"

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QLocale>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>

EnvironmentPrefs::EnvironmentPrefs(QWidget *parent)
    : QDialog(parent)
    , categoryList_(nullptr)
    , contentStack_(nullptr)
    , environmentPage_(nullptr)
    , languageCombo_(nullptr)
    , decimalGroup_(nullptr)
    , inheritDecimal_(nullptr)
    , commaDecimal_(nullptr)
    , periodDecimal_(nullptr)
    , applyButton_(nullptr)
    , okButton_(nullptr)
    , cancelButton_(nullptr)
    , settings_(new QSettings("Phoenix", "Sprint4", this))
{
    setupUI();
    loadSettings();
    
    // Set window properties
    setWindowTitle("Preferences - Phoenix IDE");
    setModal(true);
    resize(600, 400);
}

EnvironmentPrefs::~EnvironmentPrefs() = default;

void EnvironmentPrefs::setupUI()
{
    // Main layout
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    
    // Left side - category list
    categoryList_ = new QListWidget(this);
    categoryList_->setMaximumWidth(150);
    categoryList_->addItem("Environment");
    categoryList_->addItem("Editor");
    categoryList_->addItem("Analysis");
    categoryList_->addItem("Tools");
    categoryList_->addItem("Advanced");
    
    // Right side - content stack
    contentStack_ = new QStackedWidget(this);
    
    // Environment page
    environmentPage_ = new QWidget();
    QVBoxLayout* envLayout = new QVBoxLayout(environmentPage_);
    
    // Language section
    QGroupBox* languageGroup = new QGroupBox("Language & Region", environmentPage_);
    QFormLayout* languageForm = new QFormLayout(languageGroup);
    
    languageCombo_ = new QComboBox(environmentPage_);
    languageCombo_->addItem("System Default", "");
    languageCombo_->addItem("English (US)", "en_US");
    languageCombo_->addItem("English (UK)", "en_GB");
    languageCombo_->addItem("French", "fr_FR");
    languageCombo_->addItem("German", "de_DE");
    languageCombo_->addItem("Spanish", "es_ES");
    languageCombo_->addItem("Japanese", "ja_JP");
    languageCombo_->addItem("Chinese (Simplified)", "zh_CN");
    
    languageForm->addRow("Language:", languageCombo_);
    envLayout->addWidget(languageGroup);
    
    // Decimal separator section
    QGroupBox* decimalGroup = new QGroupBox("Number Format", environmentPage_);
    QVBoxLayout* decimalLayout = new QVBoxLayout(decimalGroup);
    
    decimalGroup_ = new QButtonGroup(environmentPage_);
    inheritDecimal_ = new QRadioButton("Inherit from system locale", environmentPage_);
    commaDecimal_ = new QRadioButton("Use comma (1,234.56)", environmentPage_);
    periodDecimal_ = new QRadioButton("Use period (1.234,56)", environmentPage_);
    
    decimalGroup_->addButton(inheritDecimal_, 0);
    decimalGroup_->addButton(commaDecimal_, 1);
    decimalGroup_->addButton(periodDecimal_, 2);
    
    decimalLayout->addWidget(inheritDecimal_);
    decimalLayout->addWidget(commaDecimal_);
    decimalLayout->addWidget(periodDecimal_);
    
    envLayout->addWidget(decimalGroup);
    
    // macOS-specific note (TODO for future)
    QLabel* macosNote = new QLabel("Note: On macOS, app-global menu vs in-window menu setting will be available in a future update.", environmentPage_);
    macosNote->setWordWrap(true);
    macosNote->setStyleSheet("color: gray; font-style: italic;");
    envLayout->addWidget(macosNote);
    
    envLayout->addStretch();
    
    // Add environment page to stack
    contentStack_->addWidget(environmentPage_);
    
    // Add other placeholder pages
    for (int i = 1; i < 5; ++i) {
        QWidget* page = new QWidget();
        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        QLabel* label = new QLabel(QString("Settings for category %1").arg(i + 1));
        label->setAlignment(Qt::AlignCenter);
        pageLayout->addWidget(label);
        contentStack_->addWidget(page);
    }
    
    // Connect category selection
    connect(categoryList_, &QListWidget::currentRowChanged, 
            contentStack_, &QStackedWidget::setCurrentIndex);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    applyButton_ = new QPushButton("Apply", this);
    okButton_ = new QPushButton("OK", this);
    cancelButton_ = new QPushButton("Cancel", this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton_);
    buttonLayout->addWidget(okButton_);
    buttonLayout->addWidget(cancelButton_);
    
    // Main layout
    mainLayout->addWidget(categoryList_);
    mainLayout->addWidget(contentStack_);
    
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(contentStack_);
    rightLayout->addLayout(buttonLayout);
    
    mainLayout->addLayout(rightLayout);
    
    // Connect buttons
    connect(applyButton_, &QPushButton::clicked, this, &EnvironmentPrefs::onApply);
    connect(okButton_, &QPushButton::clicked, this, &EnvironmentPrefs::onOK);
    connect(cancelButton_, &QPushButton::clicked, this, &EnvironmentPrefs::onCancel);
    
    // Connect language change
    connect(languageCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnvironmentPrefs::onLanguageChanged);
    connect(decimalGroup_, &QButtonGroup::buttonClicked,
            this, &EnvironmentPrefs::onDecimalSeparatorChanged);
}

void EnvironmentPrefs::loadSettings()
{
    // Load language setting
    QString localeName = settings_->value("locale", QLocale::system().name()).toString();
    int index = languageCombo_->findData(localeName);
    if (index >= 0) {
        languageCombo_->setCurrentIndex(index);
    } else {
        languageCombo_->setCurrentIndex(0); // System default
    }
    
    // Load decimal separator setting
    QString decimalSetting = settings_->value("decimalSeparator", "inherit").toString();
    if (decimalSetting == "comma") {
        commaDecimal_->setChecked(true);
    } else if (decimalSetting == "period") {
        periodDecimal_->setChecked(true);
    } else {
        inheritDecimal_->setChecked(true);
    }
    
    currentLocale_ = QLocale(localeName);
    currentDecimalSeparator_ = decimalSetting;
}

void EnvironmentPrefs::saveSettings()
{
    // Save language setting
    QString localeName = languageCombo_->currentData().toString();
    if (localeName.isEmpty()) {
        localeName = QLocale::system().name();
    }
    settings_->setValue("locale", localeName);
    
    // Save decimal separator setting
    QString decimalSetting;
    if (commaDecimal_->isChecked()) {
        decimalSetting = "comma";
    } else if (periodDecimal_->isChecked()) {
        decimalSetting = "period";
    } else {
        decimalSetting = "inherit";
    }
    settings_->setValue("decimalSeparator", decimalSetting);
}

void EnvironmentPrefs::updateLocale()
{
    // Update application locale
    QString localeName = languageCombo_->currentData().toString();
    if (!localeName.isEmpty()) {
        QLocale newLocale(localeName);
        QLocale::setDefault(newLocale);
        currentLocale_ = newLocale;
    }
}

void EnvironmentPrefs::onLanguageChanged()
{
    updateLocale();
}

void EnvironmentPrefs::onDecimalSeparatorChanged()
{
    // Decimal separator change doesn't need immediate effect
    // It will be applied when settings are saved
}

void EnvironmentPrefs::onApply()
{
    saveSettings();
    updateLocale();
    
    // Emit signal to notify main window of changes
    // TODO: Implement signal/slot for settings change notification
    
    QMessageBox::information(this, "Settings Applied", 
                            "Environment settings have been applied.");
}

void EnvironmentPrefs::onOK()
{
    onApply();
    accept();
}

void EnvironmentPrefs::onCancel()
{
    // Reload settings to discard changes
    loadSettings();
    reject();
}

void EnvironmentPrefs::closeEvent(QCloseEvent *event)
{
    // Ask user if they want to save changes
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Save Changes?",
        "Do you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );
    
    if (reply == QMessageBox::Save) {
        onApply();
        event->accept();
    } else if (reply == QMessageBox::Discard) {
        event->accept();
    } else {
        event->ignore();
    }
}

#include "EnvironmentPrefs.moc"
