#pragma once

#include "Message.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QImageReader>
#include <QPixmap>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <memory>
#include <vector>

QT_BEGIN_NAMESPACE
class QProgressDialog;
QT_END_NAMESPACE

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);
    ~FileManager();
    
    // File operations
    QString openFileDialog(const QString& title = "Open File", 
                          const QString& filter = "All Files (*.*)");
    QStringList openFilesDialog(const QString& title = "Open Files", 
                               const QString& filter = "All Files (*.*)");
    QString saveFileDialog(const QString& title = "Save File", 
                          const QString& defaultName = "",
                          const QString& filter = "All Files (*.*)");
    
    // Attachment handling
    std::shared_ptr<Attachment> createAttachment(const QString& filePath);
    bool validateFile(const QString& filePath, QString* errorMessage = nullptr);
    bool isImageFile(const QString& filePath) const;
    bool isTextFile(const QString& filePath) const;
    
    // Image processing
    QPixmap loadImage(const QString& filePath, const QSize& maxSize = QSize(800, 600));
    QPixmap loadImageFromData(const QByteArray& data, const QSize& maxSize = QSize(800, 600));
    QByteArray compressImage(const QPixmap& pixmap, int quality = 85);
    
    // Text file processing
    QString loadTextFile(const QString& filePath, const QString& encoding = "UTF-8");
    bool saveTextFile(const QString& filePath, const QString& content, const QString& encoding = "UTF-8");
    
    // Conversation file operations
    bool saveConversation(const QString& filePath, const std::vector<Message>& messages);
    bool loadConversation(const QString& filePath, std::vector<Message>& messages);
    bool exportMarkdown(const QString& filePath, const std::vector<Message>& messages);
    bool exportHTML(const QString& filePath, const std::vector<Message>& messages);
    
    // Configuration
    void setMaxFileSize(int bytes) { m_maxFileSize = bytes; }
    void setAllowedImageTypes(const QStringList& types) { m_allowedImageTypes = types; }
    void setAllowedFileTypes(const QStringList& types) { m_allowedFileTypes = types; }
    
    // Getters
    int getMaxFileSize() const { return m_maxFileSize; }
    QStringList getAllowedImageTypes() const { return m_allowedImageTypes; }
    QStringList getAllowedFileTypes() const { return m_allowedFileTypes; }
    
    // Utility
    QString getFileExtension(const QString& filePath) const;
    QString getMimeType(const QString& filePath) const;
    QString formatFileSize(qint64 bytes) const;
    bool isValidExtension(const QString& filePath, const QStringList& allowedTypes) const;
    
    // Paths
    QString getAppDataPath() const;
    QString getConversationsPath() const;
    QString getExportsPath() const;
    QString getCachePath() const;

signals:
    void fileOperationStarted(const QString& operation);
    void fileOperationProgress(int percentage);
    void fileOperationCompleted(bool success, const QString& message);
    void attachmentCreated(std::shared_ptr<Attachment> attachment);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    void initializePaths();
    bool createDirectoryIfNotExists(const QString& path);
    QByteArray readFileData(const QString& filePath);
    bool writeFileData(const QString& filePath, const QByteArray& data);
    
    // JSON conversion helpers
    QJsonObject messageToJson(const Message& message) const;
    Message messageFromJson(const QJsonObject& json) const;
    QJsonObject attachmentToJson(const Attachment& attachment) const;
    std::shared_ptr<Attachment> attachmentFromJson(const QJsonObject& json) const;
    
    // HTML export helpers
    QString generateHTMLHeader() const;
    QString generateHTMLFooter() const;
    QString messageToHTML(const Message& message) const;
    
    // Markdown export helpers
    QString messageToMarkdown(const Message& message) const;
    QString escapeMarkdown(const QString& text) const;
    
    QMimeDatabase m_mimeDatabase;
    QNetworkAccessManager* m_networkManager;
    QProgressDialog* m_progressDialog = nullptr;
    
    // Configuration
    int m_maxFileSize = 10 * 1024 * 1024; // 10MB default
    QStringList m_allowedImageTypes = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"};
    QStringList m_allowedFileTypes = {".txt", ".md", ".cpp", ".h", ".py", ".js", ".json", ".xml", ".csv"};
    
    // Paths
    QString m_appDataPath;
    QString m_conversationsPath;
    QString m_exportsPath;
    QString m_cachePath;
    
    // State
    QTimer* m_operationTimer;
    QString m_currentOperation;
}; 