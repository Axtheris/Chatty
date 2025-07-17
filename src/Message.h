#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QByteArray>
#include <vector>
#include <memory>

enum class MessageRole {
    User,
    Assistant,
    System
};

enum class MessageStatus {
    Sending,
    Streaming,
    Complete,
    Error
};

struct Attachment {
    QString filename;
    QString filepath;
    QString mimeType;
    QByteArray data;
    bool isImage;
    
    Attachment(const QString& file, const QString& path, const QString& mime, bool img = false)
        : filename(file), filepath(path), mimeType(mime), isImage(img) {}
};

struct Message {
    QString id;
    QString content;
    MessageRole role;
    MessageStatus status;
    QDateTime timestamp;
    std::vector<std::shared_ptr<Attachment>> attachments;
    
    // Streaming metadata
    int totalTokens = 0;
    double tokensPerSecond = 0.0;
    QDateTime streamStartTime;
    QDateTime streamEndTime;
    
    // UI state
    bool isExpanded = true;
    float animationProgress = 0.0f;
    
    Message(const QString& text, MessageRole r)
        : content(text), role(r), status(MessageStatus::Complete), timestamp(QDateTime::currentDateTime()) {
        generateId();
    }
    
    Message() : role(MessageRole::User), status(MessageStatus::Complete), timestamp(QDateTime::currentDateTime()) {
        generateId();
    }
    
    void generateId() {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        id = QString("msg_%1").arg(now);
    }
    
    void addAttachment(std::shared_ptr<Attachment> attachment) {
        attachments.push_back(attachment);
    }
    
    void startStreaming() {
        status = MessageStatus::Streaming;
        streamStartTime = QDateTime::currentDateTime();
        totalTokens = 0;
        tokensPerSecond = 0.0;
    }
    
    void updateStreaming(const QString& newContent) {
        content = newContent;
        totalTokens = static_cast<int>(content.length() / 4); // Rough token estimate
        
        QDateTime now = QDateTime::currentDateTime();
        qint64 duration = streamStartTime.msecsTo(now);
        if (duration > 0) {
            tokensPerSecond = (totalTokens * 1000.0) / duration;
        }
    }
    
    void completeStreaming() {
        status = MessageStatus::Complete;
        streamEndTime = QDateTime::currentDateTime();
    }
    
    void setError() {
        status = MessageStatus::Error;
    }
    
    bool isFromUser() const {
        return role == MessageRole::User;
    }
    
    bool isFromAssistant() const {
        return role == MessageRole::Assistant;
    }
    
    bool isSystemMessage() const {
        return role == MessageRole::System;
    }
    
    QString getFormattedTime() const {
        return timestamp.toString("hh:mm:ss");
    }
}; 