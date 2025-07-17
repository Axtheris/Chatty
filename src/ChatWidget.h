#pragma once

#include "Message.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>
#include <QTimer>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <vector>
#include <memory>

class OpenRouterAPI;
class MessageWidget;
class MarkdownRenderer;
class FileManager;

QT_BEGIN_NAMESPACE
class QSplitter;
class QListWidget;
class QListWidgetItem;
QT_END_NAMESPACE

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(OpenRouterAPI *api, FileManager *fileManager, QWidget *parent = nullptr);
    ~ChatWidget();

    // Message management
    void addMessage(const Message &message);
    void clearHistory();
    void loadHistory();
    void saveHistory();
    
    // File operations
    void saveConversation(const QString &filename);
    void loadConversation(const QString &filename);
    void exportMarkdown(const QString &filename);
    
    // UI state
    void focusInput();
    bool isInputFocused() const;
    
    // Statistics
    int getTotalMessages() const { return m_messages.size(); }
    int getTotalTokens() const;
    double getAverageTokensPerSecond() const;

signals:
    void messageAdded(const Message &message);
    void conversationChanged();
    void tokenStatsChanged(int tokens, double tps);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void sendMessage();
    void onInputTextChanged();
    void onStreamReceived(const QString &content);
    void onStreamCompleted(bool success);
    void onStreamError(const QString &error);
    void scrollToBottom();
    void updateTypingIndicator();
    void clearAttachments();

private:
    void setupUI();
    void setupMessageArea();
    void setupInputArea();
    void setupAttachmentArea();
    void addAttachment(const QString &filePath);
    void removeAttachment(int index);
    void processInput();
    void updateSendButton();
    void animateNewMessage();
    void updateTokenStats();
    
    // Message rendering
    void renderMessages();
    MessageWidget* createMessageWidget(const Message &message);
    void updateMessageWidget(MessageWidget *widget, const Message &message);
    
    // Core components
    OpenRouterAPI *m_api;
    FileManager *m_fileManager;
    std::unique_ptr<MarkdownRenderer> m_markdownRenderer;
    
    // Message data
    std::vector<Message> m_messages;
    Message m_currentMessage;
    
    // UI components - Main layout
    QVBoxLayout *m_mainLayout;
    QSplitter *m_mainSplitter;
    
    // Header area
    QFrame *m_headerFrame;
    QHBoxLayout *m_headerLayout;
    QLabel *m_chatTitleLabel;
    QLabel *m_modelLabel;
    QPushButton *m_newChatButton;
    
    // Message area
    QScrollArea *m_messageScrollArea;
    QWidget *m_messageContainer;
    QVBoxLayout *m_messageLayout;
    std::vector<MessageWidget*> m_messageWidgets;
    
    // Welcome area (shown when no messages)
    QFrame *m_welcomeFrame;
    QVBoxLayout *m_welcomeLayout;
    QLabel *m_welcomeTitleLabel;
    QLabel *m_welcomeSubtitleLabel;
    QFrame *m_suggestionsFrame;
    
    // Input area
    QFrame *m_inputFrame;
    QVBoxLayout *m_inputLayout;
    QHBoxLayout *m_inputControlsLayout;
    QTextEdit *m_inputTextEdit;
    QPushButton *m_sendButton;
    QPushButton *m_attachButton;
    QPushButton *m_clearButton;
    
    // Attachment area
    QFrame *m_attachmentFrame;
    QHBoxLayout *m_attachmentLayout;
    std::vector<std::shared_ptr<Attachment>> m_pendingAttachments;
    std::vector<QLabel*> m_attachmentLabels;
    
    // Status indicators
    QLabel *m_typingIndicator;
    QProgressBar *m_streamProgress;
    QLabel *m_tokenCountLabel;
    QTimer *m_typingTimer;
    QTimer *m_animationTimer;
    
    // State
    bool m_isStreaming = false;
    bool m_autoScroll = true;
    Message *m_streamingMessage = nullptr;
    
    // Animation
    int m_animationStep = 0;
    float m_scrollAnimation = 0.0f;
    
    // Performance tracking
    QTimer *m_statsTimer;
    std::chrono::steady_clock::time_point m_lastStatsUpdate;
    int m_tokensSinceLastUpdate = 0;
}; 