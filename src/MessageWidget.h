#pragma once

#include "Message.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QFrame>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>

class MarkdownRenderer;

QT_BEGIN_NAMESPACE
class QScrollArea;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

class MessageWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ windowOpacity WRITE setWindowOpacity)

public:
    explicit MessageWidget(const Message& message, MarkdownRenderer* renderer, QWidget *parent = nullptr);
    ~MessageWidget();
    
    void updateMessage(const Message& message);
    void setAnimated(bool animated);
    void startFadeInAnimation();
    
    const Message& getMessage() const { return m_message; }
    MessageRole getRole() const { return m_message.role; }

signals:
    void copyRequested(const QString& text);
    void retryRequested(const QString& messageId);
    void deleteRequested(const QString& messageId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void onCopyClicked();
    void onRetryClicked();
    void onDeleteClicked();
    void updateStreamingAnimation();

private:
    void setupUI();
    void setupUserMessage();
    void setupAssistantMessage();
    void setupSystemMessage();
    void updateContent();
    void updateTimestamp();
    void updateTokenStats();
    void updateAttachments();
    void applyTheme();
    
    // Layout and styling
    void styleAsUserMessage();
    void styleAsAssistantMessage();
    void styleAsSystemMessage();
    QColor getBackgroundColor() const;
    QColor getTextColor() const;
    QColor getBorderColor() const;
    
    Message m_message;
    MarkdownRenderer* m_markdownRenderer;
    
    // UI components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;
    QHBoxLayout* m_footerLayout;
    
    // Header components
    QLabel* m_avatarLabel;
    QLabel* m_nameLabel;
    QLabel* m_timestampLabel;
    QLabel* m_statusLabel;
    
    // Content area
    QFrame* m_contentFrame;
    QVBoxLayout* m_contentLayout;
    QTextEdit* m_contentTextEdit;
    QWidget* m_attachmentWidget;
    QVBoxLayout* m_attachmentLayout;
    
    // Footer components
    QFrame* m_footerFrame;
    QLabel* m_tokenStatsLabel;
    QProgressBar* m_streamProgress;
    QPushButton* m_copyButton;
    QPushButton* m_retryButton;
    QPushButton* m_deleteButton;
    
    // Animation
    QPropertyAnimation* m_fadeAnimation;
    QTimer* m_streamingTimer;
    QGraphicsOpacityEffect* m_opacityEffect;
    
    // State
    bool m_isHovered = false;
    bool m_animated = true;
    int m_streamingDots = 0;
    
    // Constants
    static constexpr int AVATAR_SIZE = 40;
    static constexpr int CONTENT_MARGIN = 16;
    static constexpr int BORDER_RADIUS = 12;
    static constexpr int CARD_SHADOW = 2;
}; 