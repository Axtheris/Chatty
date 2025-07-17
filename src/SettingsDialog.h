#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>

class Settings;
class OpenRouterAPI;

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Settings* settings, OpenRouterAPI* api, QWidget *parent = nullptr);
    ~SettingsDialog();

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onAPIKeyChanged();
    void onModelChanged();
    void onTestConnection();
    void onConnectionTested(bool success);
    void onImportSettings();
    void onExportSettings();
    void onResetSettings();
    void onFontBrowse();
    void onCodeFontBrowse();
    void onThemeChanged();
    void onModelsRefreshed(bool success);
    void validateInput();

private:
    void setupUI();
    void setupGeneralTab();
    void setupAPITab();
    void setupUITab();
    void setupChatTab();
    void setupFilesTab();
    void setupAdvancedTab();
    void setupShortcutsTab();
    
    void loadSettings();
    void saveSettings();
    void applyTheme();
    void updateModelList();
    void validateForm();
    
    Settings* m_settings;
    OpenRouterAPI* m_api;
    
    // Main layout
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    QDialogButtonBox* m_buttonBox;
    
    // General tab
    QWidget* m_generalTab;
    QCheckBox* m_darkModeCheckBox;
    QSpinBox* m_fontSizeSpinBox;
    QPushButton* m_fontButton;
    QPushButton* m_codeFontButton;
    QDoubleSpinBox* m_uiScaleSpinBox;
    QLabel* m_fontLabel;
    QLabel* m_codeFontLabel;
    
    // API tab
    QWidget* m_apiTab;
    QLineEdit* m_apiKeyLineEdit;
    QComboBox* m_modelComboBox;
    QLineEdit* m_baseURLLineEdit;
    QPushButton* m_testConnectionButton;
    QPushButton* m_refreshModelsButton;
    QLabel* m_connectionStatusLabel;
    QProgressBar* m_connectionProgress;
    
    // UI tab
    QWidget* m_uiTab;
    QCheckBox* m_showTokenStatsCheckBox;
    QCheckBox* m_autoScrollCheckBox;
    QCheckBox* m_showTimestampsCheckBox;
    QCheckBox* m_enableSoundCheckBox;
    
    // Chat tab
    QWidget* m_chatTab;
    QSpinBox* m_maxHistorySpinBox;
    QCheckBox* m_saveHistoryCheckBox;
    
    // Files tab
    QWidget* m_filesTab;
    QSpinBox* m_maxFileSizeSpinBox;
    QListWidget* m_imageTypesListWidget;
    QListWidget* m_fileTypesListWidget;
    QPushButton* m_addImageTypeButton;
    QPushButton* m_removeImageTypeButton;
    QPushButton* m_addFileTypeButton;
    QPushButton* m_removeFileTypeButton;
    
    // Advanced tab
    QWidget* m_advancedTab;
    QSpinBox* m_timeoutSpinBox;
    QSpinBox* m_retriesSpinBox;
    QCheckBox* m_enableLoggingCheckBox;
    QComboBox* m_logLevelComboBox;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_resetButton;
    
    // Shortcuts tab
    QWidget* m_shortcutsTab;
    QTreeWidget* m_shortcutsTreeWidget;
    QPushButton* m_resetShortcutsButton;
    
    // State
    bool m_settingsChanged = false;
    QTimer* m_validationTimer;
}; 