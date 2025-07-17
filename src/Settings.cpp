#include "Settings.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QCryptographicHash>
#include <QByteArray>

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    // Initialize QSettings
    QCoreApplication::setOrganizationName("Chatty Team");
    QCoreApplication::setOrganizationDomain("chatty.ai");
    QCoreApplication::setApplicationName("Chatty");
    
    m_qsettings = new QSettings(this);
    
    // Ensure config directory exists
    QString configPath = getSettingsPath();
    QDir().mkpath(QFileInfo(configPath).path());
}

Settings::~Settings() = default;

bool Settings::Load()
{
    try {
        loadFromQSettings();
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Failed to load settings:" << e.what();
        return false;
    }
}

bool Settings::Save()
{
    try {
        saveToQSettings();
        m_qsettings->sync();
        emit settingsChanged();
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Failed to save settings:" << e.what();
        return false;
    }
}

void Settings::Reset()
{
    m_settings = AppSettings(); // Reset to defaults
    emit settingsChanged();
}

void Settings::SetAPIKey(const QString& key)
{
    if (m_settings.apiKey != key) {
        m_settings.apiKey = key;
        emit apiKeyChanged(key);
        emit settingsChanged();
    }
}

void Settings::SetSelectedModel(const QString& model)
{
    if (m_settings.selectedModel != model) {
        m_settings.selectedModel = model;
        emit modelChanged(model);
        emit settingsChanged();
    }
}

void Settings::SetDarkMode(bool dark)
{
    if (m_settings.darkMode != dark) {
        m_settings.darkMode = dark;
        emit themeChanged(dark);
        emit settingsChanged();
    }
}

void Settings::SetFontSize(int size)
{
    if (m_settings.fontSize != size) {
        m_settings.fontSize = size;
        emit settingsChanged();
    }
}

bool Settings::ValidateAPIKey(const QString& key) const
{
    // Basic validation - should be a non-empty string
    // OpenRouter API keys typically start with "sk-or-"
    if (key.isEmpty()) {
        return false;
    }
    
    // Check minimum length (typical API keys are at least 32 characters)
    if (key.length() < 10) {
        return false;
    }
    
    // Could add more sophisticated validation here
    return true;
}

bool Settings::ValidateModel(const QString& model) const
{
    // Basic validation - should be in format "provider/model"
    if (model.isEmpty()) {
        return false;
    }
    
    // Check if it contains a slash (provider/model format)
    return model.contains('/');
}

bool Settings::ValidateFileType(const QString& filename, bool isImage) const
{
    if (isImage) {
        return isValidFileExtension(filename, m_settings.allowedImageTypes);
    } else {
        return isValidFileExtension(filename, m_settings.allowedFileTypes);
    }
}

bool Settings::ImportSettings(const QString& filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open settings file for import:" << filepath;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse settings JSON:" << error.errorString();
        return false;
    }
    
    QJsonObject obj = doc.object();
    
    // Import settings from JSON
    AppSettings importedSettings;
    
    // API Configuration
    if (obj.contains("apiKey")) {
        importedSettings.apiKey = obj["apiKey"].toString();
    }
    if (obj.contains("selectedModel")) {
        importedSettings.selectedModel = obj["selectedModel"].toString();
    }
    if (obj.contains("baseURL")) {
        importedSettings.baseURL = obj["baseURL"].toString();
    }
    
    // UI Preferences
    if (obj.contains("darkMode")) {
        importedSettings.darkMode = obj["darkMode"].toBool();
    }
    if (obj.contains("fontSize")) {
        importedSettings.fontSize = obj["fontSize"].toInt();
    }
    if (obj.contains("uiScale")) {
        importedSettings.uiScale = obj["uiScale"].toDouble();
    }
    
    // Apply imported settings
    m_settings = importedSettings;
    emit settingsChanged();
    
    return true;
}

bool Settings::ExportSettings(const QString& filepath)
{
    QJsonObject obj;
    
    // Export current settings to JSON (excluding sensitive data like API key)
    obj["selectedModel"] = m_settings.selectedModel;
    obj["baseURL"] = m_settings.baseURL;
    obj["darkMode"] = m_settings.darkMode;
    obj["fontSize"] = m_settings.fontSize;
    obj["uiScale"] = m_settings.uiScale;
    obj["showTokenStats"] = m_settings.showTokenStats;
    obj["autoScroll"] = m_settings.autoScroll;
    obj["showTimestamps"] = m_settings.showTimestamps;
    obj["maxHistoryMessages"] = m_settings.maxHistoryMessages;
    obj["saveHistory"] = m_settings.saveHistory;
    
    QJsonDocument doc(obj);
    
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for export:" << filepath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

QString Settings::getSettingsPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.json";
}

QString Settings::encryptAPIKey(const QString& key) const
{
    // Simple obfuscation - in a real application, use proper encryption
    QByteArray data = key.toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString::fromLatin1(data.toBase64()) + ":" + QString::fromLatin1(hash.toHex());
}

QString Settings::decryptAPIKey(const QString& encryptedKey) const
{
    // Simple de-obfuscation
    QStringList parts = encryptedKey.split(':');
    if (parts.size() != 2) {
        return encryptedKey; // Return as-is if not encrypted format
    }
    
    QByteArray data = QByteArray::fromBase64(parts[0].toLatin1());
    QByteArray expectedHash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    QByteArray actualHash = QByteArray::fromHex(parts[1].toLatin1());
    
    if (expectedHash == actualHash) {
        return QString::fromUtf8(data);
    }
    
    return QString(); // Return empty if hash doesn't match
}

bool Settings::createDefaultConfig()
{
    // The default constructor already sets up defaults
    return Save();
}

void Settings::loadFromQSettings()
{
    m_qsettings->beginGroup("API");
    m_settings.apiKey = decryptAPIKey(m_qsettings->value("apiKey", "").toString());
    m_settings.selectedModel = m_qsettings->value("selectedModel", m_settings.selectedModel).toString();
    m_settings.baseURL = m_qsettings->value("baseURL", m_settings.baseURL).toString();
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("UI");
    m_settings.darkMode = m_qsettings->value("darkMode", m_settings.darkMode).toBool();
    m_settings.fontSize = m_qsettings->value("fontSize", m_settings.fontSize).toInt();
    m_settings.fontPath = m_qsettings->value("fontPath", m_settings.fontPath).toString();
    m_settings.codeFontPath = m_qsettings->value("codeFontPath", m_settings.codeFontPath).toString();
    m_settings.uiScale = m_qsettings->value("uiScale", m_settings.uiScale).toDouble();
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Chat");
    m_settings.showTokenStats = m_qsettings->value("showTokenStats", m_settings.showTokenStats).toBool();
    m_settings.autoScroll = m_qsettings->value("autoScroll", m_settings.autoScroll).toBool();
    m_settings.showTimestamps = m_qsettings->value("showTimestamps", m_settings.showTimestamps).toBool();
    m_settings.enableSoundNotifications = m_qsettings->value("enableSoundNotifications", m_settings.enableSoundNotifications).toBool();
    m_settings.maxHistoryMessages = m_qsettings->value("maxHistoryMessages", m_settings.maxHistoryMessages).toInt();
    m_settings.saveHistory = m_qsettings->value("saveHistory", m_settings.saveHistory).toBool();
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Files");
    m_settings.maxFileSize = m_qsettings->value("maxFileSize", m_settings.maxFileSize).toInt();
    m_settings.allowedImageTypes = m_qsettings->value("allowedImageTypes", m_settings.allowedImageTypes).toStringList();
    m_settings.allowedFileTypes = m_qsettings->value("allowedFileTypes", m_settings.allowedFileTypes).toStringList();
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Advanced");
    m_settings.requestTimeout = m_qsettings->value("requestTimeout", m_settings.requestTimeout).toInt();
    m_settings.maxRetries = m_qsettings->value("maxRetries", m_settings.maxRetries).toInt();
    m_settings.enableLogging = m_qsettings->value("enableLogging", m_settings.enableLogging).toBool();
    m_settings.logLevel = m_qsettings->value("logLevel", m_settings.logLevel).toString();
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Window");
    m_settings.windowSize = m_qsettings->value("windowSize", m_settings.windowSize).toSize();
    m_settings.windowPosition = m_qsettings->value("windowPosition", m_settings.windowPosition).toPoint();
    m_settings.maximized = m_qsettings->value("maximized", m_settings.maximized).toBool();
    m_settings.rememberWindowState = m_qsettings->value("rememberWindowState", m_settings.rememberWindowState).toBool();
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Shortcuts");
    m_settings.shortcuts = m_qsettings->value("shortcuts", m_settings.shortcuts).toMap();
    m_qsettings->endGroup();
}

void Settings::saveToQSettings()
{
    m_qsettings->beginGroup("API");
    m_qsettings->setValue("apiKey", encryptAPIKey(m_settings.apiKey));
    m_qsettings->setValue("selectedModel", m_settings.selectedModel);
    m_qsettings->setValue("baseURL", m_settings.baseURL);
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("UI");
    m_qsettings->setValue("darkMode", m_settings.darkMode);
    m_qsettings->setValue("fontSize", m_settings.fontSize);
    m_qsettings->setValue("fontPath", m_settings.fontPath);
    m_qsettings->setValue("codeFontPath", m_settings.codeFontPath);
    m_qsettings->setValue("uiScale", m_settings.uiScale);
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Chat");
    m_qsettings->setValue("showTokenStats", m_settings.showTokenStats);
    m_qsettings->setValue("autoScroll", m_settings.autoScroll);
    m_qsettings->setValue("showTimestamps", m_settings.showTimestamps);
    m_qsettings->setValue("enableSoundNotifications", m_settings.enableSoundNotifications);
    m_qsettings->setValue("maxHistoryMessages", m_settings.maxHistoryMessages);
    m_qsettings->setValue("saveHistory", m_settings.saveHistory);
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Files");
    m_qsettings->setValue("maxFileSize", m_settings.maxFileSize);
    m_qsettings->setValue("allowedImageTypes", m_settings.allowedImageTypes);
    m_qsettings->setValue("allowedFileTypes", m_settings.allowedFileTypes);
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Advanced");
    m_qsettings->setValue("requestTimeout", m_settings.requestTimeout);
    m_qsettings->setValue("maxRetries", m_settings.maxRetries);
    m_qsettings->setValue("enableLogging", m_settings.enableLogging);
    m_qsettings->setValue("logLevel", m_settings.logLevel);
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Window");
    m_qsettings->setValue("windowSize", m_settings.windowSize);
    m_qsettings->setValue("windowPosition", m_settings.windowPosition);
    m_qsettings->setValue("maximized", m_settings.maximized);
    m_qsettings->setValue("rememberWindowState", m_settings.rememberWindowState);
    m_qsettings->endGroup();
    
    m_qsettings->beginGroup("Shortcuts");
    m_qsettings->setValue("shortcuts", m_settings.shortcuts);
    m_qsettings->endGroup();
}

bool Settings::isValidFileExtension(const QString& filename, const QStringList& allowedTypes) const
{
    QString extension = getFileExtension(filename);
    return allowedTypes.contains(extension, Qt::CaseInsensitive);
}

QString Settings::getFileExtension(const QString& filename) const
{
    QFileInfo fileInfo(filename);
    QString extension = fileInfo.suffix();
    if (!extension.isEmpty()) {
        extension = "." + extension.toLower();
    }
    return extension;
} 