#include "FileManager.h"
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>

FileManager::FileManager(QObject* parent)
    : QObject(parent)
    , m_maxFileSize(10 * 1024 * 1024)  // 10MB default
    , m_imageQuality(85)
    , m_maxImageDimension(2048)
{
    // Initialize supported file types
    initializeSupportedTypes();
    
    // Ensure cache directory exists
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir + "/chatty_files");
}

void FileManager::initializeSupportedTypes()
{
    // Supported image formats
    m_supportedImageTypes = {
        "image/jpeg", "image/jpg", "image/png", "image/gif", 
        "image/bmp", "image/webp", "image/svg+xml"
    };
    
    // Supported document formats
    m_supportedDocumentTypes = {
        "text/plain", "text/markdown", "text/csv",
        "application/pdf", "application/json", "application/xml",
        "application/msword", "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "application/vnd.ms-excel", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
        "application/vnd.ms-powerpoint", "application/vnd.openxmlformats-officedocument.presentationml.presentation"
    };
    
    // Supported code file types
    m_supportedCodeTypes = {
        "text/x-c", "text/x-cpp", "text/x-java", "text/x-python", "text/x-scala",
        "text/javascript", "text/typescript", "text/css", "text/html", "text/xml"
    };
}

QString FileManager::openFileDialog(QWidget* parent, const QString& title)
{
    QString filter = createFileFilter();
    
    QString fileName = QFileDialog::getOpenFileName(
        parent,
        title.isEmpty() ? "Select File to Upload" : title,
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        filter
    );
    
    return fileName;
}

QStringList FileManager::openMultipleFilesDialog(QWidget* parent, const QString& title)
{
    QString filter = createFileFilter();
    
    QStringList fileNames = QFileDialog::getOpenFileNames(
        parent,
        title.isEmpty() ? "Select Files to Upload" : title,
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        filter
    );
    
    return fileNames;
}

MessageAttachment FileManager::processFile(const QString& filePath)
{
    MessageAttachment attachment;
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        emit fileProcessingError("File does not exist or is not readable: " + filePath);
        return attachment;
    }
    
    // Check file size
    if (fileInfo.size() > m_maxFileSize) {
        emit fileProcessingError(QString("File size (%1) exceeds maximum allowed size (%2)")
                                .arg(formatFileSize(fileInfo.size()))
                                .arg(formatFileSize(m_maxFileSize)));
        return attachment;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit fileProcessingError("Failed to open file: " + filePath);
        return attachment;
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    // Detect MIME type
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFileNameAndData(fileInfo.fileName(), fileData);
    QString mimeTypeName = mimeType.name();
    
    // Validate file type
    if (!isFileTypeSupported(mimeTypeName)) {
        emit fileProcessingError("Unsupported file type: " + mimeTypeName);
        return attachment;
    }
    
    // Process based on file type
    if (isImageFile(mimeTypeName)) {
        fileData = processImage(fileData, mimeTypeName);
        attachment.setType("image");
    } else if (isDocumentFile(mimeTypeName) || isCodeFile(mimeTypeName)) {
        attachment.setType("document");
    } else {
        attachment.setType("file");
    }
    
    // Set attachment properties
    attachment.setFilename(fileInfo.fileName());
    attachment.setMimeType(mimeTypeName);
    attachment.setData(fileData);
    attachment.setId(generateFileId(fileData));
    
    emit fileProcessed(attachment);
    return attachment;
}

QList<MessageAttachment> FileManager::processFiles(const QStringList& filePaths)
{
    QList<MessageAttachment> attachments;
    
    for (const QString& filePath : filePaths) {
        MessageAttachment attachment = processFile(filePath);
        if (!attachment.filename().isEmpty()) {
            attachments.append(attachment);
        }
    }
    
    return attachments;
}

QByteArray FileManager::processImage(const QByteArray& imageData, const QString& mimeType)
{
    QBuffer buffer;
    buffer.setData(imageData);
    buffer.open(QIODevice::ReadOnly);
    
    QImageReader reader(&buffer);
    QImage image = reader.read();
    
    if (image.isNull()) {
        qWarning() << "Failed to load image data";
        return imageData;  // Return original data if processing fails
    }
    
    // Resize if too large
    if (image.width() > m_maxImageDimension || image.height() > m_maxImageDimension) {
        image = image.scaled(m_maxImageDimension, m_maxImageDimension, 
                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // Convert to JPEG for compression (unless it's PNG with transparency)
    QByteArray processedData;
    QBuffer outputBuffer(&processedData);
    outputBuffer.open(QIODevice::WriteOnly);
    
    QString format = "JPEG";
    if (mimeType == "image/png" && image.hasAlphaChannel()) {
        format = "PNG";
    }
    
    QImageWriter writer(&outputBuffer, format.toUtf8());
    writer.setQuality(m_imageQuality);
    
    if (!writer.write(image)) {
        qWarning() << "Failed to compress image";
        return imageData;  // Return original data if compression fails
    }
    
    return processedData;
}

bool FileManager::isFileTypeSupported(const QString& mimeType) const
{
    return isImageFile(mimeType) || isDocumentFile(mimeType) || isCodeFile(mimeType);
}

bool FileManager::isImageFile(const QString& mimeType) const
{
    return m_supportedImageTypes.contains(mimeType);
}

bool FileManager::isDocumentFile(const QString& mimeType) const
{
    return m_supportedDocumentTypes.contains(mimeType);
}

bool FileManager::isCodeFile(const QString& mimeType) const
{
    return m_supportedCodeTypes.contains(mimeType);
}

QString FileManager::createFileFilter() const
{
    QStringList filters;
    
    // All supported files
    QStringList allExtensions;
    
    // Image files
    QStringList imageExtensions = {"*.jpg", "*.jpeg", "*.png", "*.gif", "*.bmp", "*.webp", "*.svg"};
    allExtensions.append(imageExtensions);
    filters.append("Image Files (" + imageExtensions.join(" ") + ")");
    
    // Document files
    QStringList docExtensions = {"*.txt", "*.md", "*.csv", "*.pdf", "*.json", "*.xml", 
                                "*.doc", "*.docx", "*.xls", "*.xlsx", "*.ppt", "*.pptx"};
    allExtensions.append(docExtensions);
    filters.append("Document Files (" + docExtensions.join(" ") + ")");
    
    // Code files
    QStringList codeExtensions = {"*.c", "*.cpp", "*.h", "*.hpp", "*.java", "*.py", "*.scala",
                                 "*.js", "*.ts", "*.css", "*.html", "*.htm", "*.xml"};
    allExtensions.append(codeExtensions);
    filters.append("Code Files (" + codeExtensions.join(" ") + ")");
    
    // Add "All Supported Files" at the beginning
    filters.prepend("All Supported Files (" + allExtensions.join(" ") + ")");
    
    // Add "All Files" at the end
    filters.append("All Files (*.*)");
    
    return filters.join(";;");
}

QString FileManager::generateFileId(const QByteArray& fileData) const
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(fileData);
    return hash.result().toHex();
}

QString FileManager::formatFileSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString::number(bytes / GB, 'f', 1) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / MB, 'f', 1) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / KB, 'f', 1) + " KB";
    } else {
        return QString::number(bytes) + " bytes";
    }
}

bool FileManager::saveFile(const QString& filePath, const QByteArray& data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit fileProcessingError("Failed to create file: " + filePath);
        return false;
    }
    
    qint64 written = file.write(data);
    file.close();
    
    if (written != data.size()) {
        emit fileProcessingError("Failed to write complete file: " + filePath);
        return false;
    }
    
    return true;
}

QString FileManager::saveAttachmentDialog(QWidget* parent, const MessageAttachment& attachment)
{
    QString defaultFileName = attachment.filename();
    QString fileName = QFileDialog::getSaveFileName(
        parent,
        "Save Attachment",
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + defaultFileName,
        "All Files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        if (saveFile(fileName, attachment.data())) {
            return fileName;
        }
    }
    
    return QString();
}

void FileManager::setMaxFileSize(qint64 maxSize)
{
    m_maxFileSize = maxSize;
}

qint64 FileManager::maxFileSize() const
{
    return m_maxFileSize;
}

void FileManager::setImageQuality(int quality)
{
    m_imageQuality = qBound(1, quality, 100);
}

int FileManager::imageQuality() const
{
    return m_imageQuality;
}

void FileManager::setMaxImageDimension(int dimension)
{
    m_maxImageDimension = qMax(100, dimension);
}

int FileManager::maxImageDimension() const
{
    return m_maxImageDimension;
}

QStringList FileManager::supportedImageTypes() const
{
    return m_supportedImageTypes;
}

QStringList FileManager::supportedDocumentTypes() const
{
    return m_supportedDocumentTypes;
}

QStringList FileManager::supportedCodeTypes() const
{
    return m_supportedCodeTypes;
} 