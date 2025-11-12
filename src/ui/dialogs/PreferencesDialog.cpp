#include "PreferencesDialog.h"
#include "EnvironmentPage.h"
#include "LanguagePage.h"
#include "../main/MainWindow.h"
#include "app/SettingsProvider.h"
#include "app/SettingsKeys.h"
#include "app/PhxConstants.h"
#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QSettings>
#include <QDebug>

PreferencesDialog::PreferencesDialog(MainWindow* mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_splitter(nullptr)
    , m_categoryList(nullptr)
    , m_contentStack(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_applyButton(nullptr)
    , m_environmentPage(nullptr)
    , m_languagePage(nullptr)
    , m_mainWindow(mainWindow)
    , m_settings(mainWindow->settingsProvider()->settings())
{
    setWindowTitle(tr("Preferences"));
    setModal(true);
    setMinimumSize(phx::ui::kPrefsMinSize);
    resize(phx::ui::kPrefsInitSize);
    
    setupUI();
    loadSettings();
}

PreferencesDialog::~PreferencesDialog()
{
    saveSettings();
}

void PreferencesDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void PreferencesDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create splitter for two-pane layout
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // Left pane - category list
    m_categoryList = new QListWidget();
    m_categoryList->setMaximumWidth(200);
    m_categoryList->setMinimumWidth(150);
    
    // Add categories
    m_categoryList->addItem(tr("Environment"));
    m_categoryList->addItem(tr("Language"));
    // Future categories can be added here
    
    m_splitter->addWidget(m_categoryList);
    
    // Right pane - content stack
    m_contentStack = new QStackedWidget();
    
    // Create pages (pass QSettings& reference)
    m_environmentPage = new EnvironmentPage(m_settings, this);
    m_contentStack->addWidget(m_environmentPage);

    connect(m_environmentPage, &EnvironmentPage::settingsReset, this, &PreferencesDialog::onSettingsReset);
    
    m_languagePage = new LanguagePage(m_settings, this);
    m_contentStack->addWidget(m_languagePage);
    
    m_splitter->addWidget(m_contentStack);
    m_splitter->setSizes({200, 600}); // Set initial sizes
    
    mainLayout->addWidget(m_splitter);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("OK"));
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &PreferencesDialog::onOK);
    buttonLayout->addWidget(m_okButton);
    
    m_cancelButton = new QPushButton(tr("Cancel"));
    connect(m_cancelButton, &QPushButton::clicked, this, &PreferencesDialog::onCancel);
    buttonLayout->addWidget(m_cancelButton);
    
    m_applyButton = new QPushButton(tr("Apply"));
    connect(m_applyButton, &QPushButton::clicked, this, &PreferencesDialog::onApply);
    buttonLayout->addWidget(m_applyButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect category selection
    connect(m_categoryList, &QListWidget::currentRowChanged, this, &PreferencesDialog::onCategoryChanged);
    
    // Set initial selection
    m_categoryList->setCurrentRow(0);
    onCategoryChanged();
}

void PreferencesDialog::onCategoryChanged()
{
    int currentRow = m_categoryList->currentRow();
    if (currentRow >= 0 && currentRow < m_contentStack->count()) {
        m_contentStack->setCurrentIndex(currentRow);
    }
}

void PreferencesDialog::onOK()
{
    saveSettings();
    accept();
}

void PreferencesDialog::onCancel()
{
    reject();
}

void PreferencesDialog::onApply()
{
    saveSettings();
}

void PreferencesDialog::onSettingsReset()
{
    if (m_languagePage) {
        m_languagePage->loadSettings();
    }
    
    // Reset toolbar/dock layout immediately
    if (auto* mainWindow = qobject_cast<MainWindow*>(parent())) {
        mainWindow->applyCanonicalLayout();
    }
}

void PreferencesDialog::loadSettings()
{
    // Load dialog geometry
    if (auto b = m_settings.value(PhxKeys::DIALOG_PREFS_GEO).toByteArray(); !b.isEmpty()) {
        restoreGeometry(b);
    }
    
    // Load page settings
    if (m_environmentPage) {
        m_environmentPage->loadSettings();
    }
    if (m_languagePage) {
        m_languagePage->loadSettings();
    }
}

void PreferencesDialog::saveSettings()
{
    // Save dialog geometry
    m_settings.setValue(PhxKeys::DIALOG_PREFS_GEO, saveGeometry());
    
    // Save page settings
    if (m_environmentPage) {
        m_environmentPage->saveSettings();
    }
    if (m_languagePage) {
        m_languagePage->saveSettings();
    }
}
