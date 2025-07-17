#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QSettings>
#include <QSize>
#include <QPoint>
#include <functional>

struct AppSettings {
    // API Configuration
    QString apiKey;
    QString selectedModel = "openai/gpt-3.5-turbo";
    QString baseURL = "https://openrouter.ai/api/v1";
    
    // UI Preferences
    bool darkMode = true;
    int fontSize = 14;
    QString fontPath;
    QString codeFontPath;
    double uiScale = 1.0;
    
    // Chat Settings
    bool showTokenStats = true;
    bool autoScroll = true;
    bool showTimestamps = true;
    bool enableSoundNotifications = false;
    int maxHistoryMessages = 1000;
    bool saveHistory = true;
    
    // File Upload Settings
    int maxFileSize = 10 * 1024 * 1024; // 10MB
    QStringList allowedImageTypes = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"};
    QStringList allowedFileTypes = {".txt", ".md", ".cpp", ".h", ".py", ".js", ".json", ".xml", ".csv"};
    
    // Advanced Settings
    int requestTimeout = 30;
    int maxRetries = 3;
    bool enableLogging = false;
    QString logLevel = "INFO";
    
    // Window Settings
    QSize windowSize = QSize(1280, 720);
    QPoint windowPosition = QPoint(-1, -1); // -1 means center
    bool maximized = false;
    bool rememberWindowState = true;
    
    // Shortcuts
    QVariantMap shortcuts = {
        {"send_message", "Return"},
        {"new_chat", "Ctrl+N"},
        {"save_chat", "Ctrl+S"},
        {"open_settings", "Ctrl+Comma"},
        {"toggle_sidebar", "Ctrl+B"}
    };
};

class Settings : public QObject {
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();
    
    // Configuration management
    bool Load();
    bool Save();
    void Reset();
    
    // Getters
    const AppSettings& GetSettings() const { return m_settings; }
    AppSettings& GetMutableSettings() { return m_settings; }
    
    // Specific getters
    const QString& GetAPIKey() const { return m_settings.apiKey; }
    const QString& GetSelectedModel() const { return m_settings.selectedModel; }
    bool IsDarkMode() const { return m_settings.darkMode; }
    int GetFontSize() const { return m_settings.fontSize; }
    
    // Specific setters
    void SetAPIKey(const QString& key);
    void SetSelectedModel(const QString& model);
    void SetDarkMode(bool dark);
    void SetFontSize(int size);
    
    // Validation
    bool ValidateAPIKey(const QString& key) const;
    bool ValidateModel(const QString& model) const;
    bool ValidateFileType(const QString& filename, bool isImage = false) const;
    
    // Import/Export
    bool ImportSettings(const QString& filepath);
    bool ExportSettings(const QString& filepath);

signals:
    void apiKeyChanged(const QString& key);
    void modelChanged(const QString& model);
    void themeChanged(bool darkMode);
    void settingsChanged();
    
private:
    AppSettings m_settings;
    QSettings *m_qsettings;
    
    // Internal methods
    QString getSettingsPath() const;
    QString encryptAPIKey(const QString& key) const;
    QString decryptAPIKey(const QString& encryptedKey) const;
    bool createDefaultConfig();
    void loadFromQSettings();
    void saveToQSettings();
    
    // Validation helpers
    bool isValidFileExtension(const QString& filename, const QStringList& allowedTypes) const;
    QString getFileExtension(const QString& filename) const;
}; 