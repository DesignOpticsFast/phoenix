#include "AnalysisPage.h"
#include "app/AnalysisRunMode.hpp"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>

AnalysisPage::AnalysisPage(QSettings& s, QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_runBehaviorGroup(nullptr)
    , m_runBehaviorLayout(nullptr)
    , m_autoRunRadio(nullptr)
    , m_showOptionsRadio(nullptr)
    , m_settings(s)
{
    setupUI();
}

AnalysisPage::~AnalysisPage()
{
}

void AnalysisPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Analysis Run Behavior Group
    m_runBehaviorGroup = new QGroupBox(tr("Analysis Run Behavior"), this);
    m_runBehaviorLayout = new QVBoxLayout(m_runBehaviorGroup);
    
    // Radio button: Auto-run
    m_autoRunRadio = new QRadioButton(
        tr("Automatically run analyses when opening an analysis window"),
        m_runBehaviorGroup);
    m_runBehaviorLayout->addWidget(m_autoRunRadio);
    
    // Radio button: Show options first
    m_showOptionsRadio = new QRadioButton(
        tr("Show parameters first; let me click Run to start"),
        m_runBehaviorGroup);
    m_runBehaviorLayout->addWidget(m_showOptionsRadio);
    
    m_mainLayout->addWidget(m_runBehaviorGroup);
    
    // Add stretch to push everything to the top
    m_mainLayout->addStretch();
}

void AnalysisPage::loadSettings()
{
    AnalysisRunMode mode = getAnalysisRunMode(m_settings);
    if (mode == AnalysisRunMode::AutoRunOnOpen) {
        m_autoRunRadio->setChecked(true);
    } else {
        m_showOptionsRadio->setChecked(true);
    }
}

void AnalysisPage::saveSettings()
{
    AnalysisRunMode mode = m_autoRunRadio->isChecked() 
        ? AnalysisRunMode::AutoRunOnOpen 
        : AnalysisRunMode::ShowOptionsFirst;
    setAnalysisRunMode(m_settings, mode);
}

