#include "ChatWidget.h"
#include "OpenRouterAPI.h"
#include "MessageWidget.h"
#include "MarkdownRenderer.h"
#include "FileManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>
#include <QTimer>
#include <QSplitter>
#include <QKeyEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QClipboard>

ChatWidget::ChatWidget(OpenRouterAPI *api, FileManager *fileManager, QWidget *parent)
    : QWidget(parent)
    , m_api(api)
    , m_fileManager(fileManager)
{
    setAcceptDrops(true);
    
    // Initialize components
    m_markdownRenderer = std::make_unique<MarkdownRenderer>(this);
    
    // Setup UI
    setupUI();
    setupMessageArea();
    setupInputArea();
    setupAttachmentArea();
    
    // Setup timers
    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    connect(m_typingTimer, &QTimer::timeout, this, &ChatWidget::updateTypingIndicator);
    
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &ChatWidget::animateNewMessage);
    
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, this, &ChatWidget::updateTokenStats);
    m_statsTimer->start(1000); // Update stats every second
    
    // Connect API signals
    if (m_api) {
        connect(m_api, &OpenRouterAPI::streamReceived, this, &ChatWidget::onStreamReceived);
        connect(m_api, &OpenRouterAPI::streamCompleted, this, &ChatWidget::onStreamCompleted);
        connect(m_api, &OpenRouterAPI::streamError, this, &ChatWidget::onStreamError);
    }
    
    // Initialize state
    m_lastStatsUpdate = std::chrono::steady_clock::now();
}

ChatWidget::~ChatWidget() = default;

void ChatWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create main splitter
    m_mainSplitter = new QSplitter(Qt::Vertical);
    m_mainLayout->addWidget(m_mainSplitter);
}

void ChatWidget::setupMessageArea()
{
    // Message scroll area
    m_messageScrollArea = new QScrollArea;
    m_messageScrollArea->setObjectName("messageScrollArea");
    m_messageScrollArea->setWidgetResizable(true);
    m_messageScrollArea->setFrameStyle(QFrame::NoFrame);
    m_messageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_messageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarNever);
    
    // Message container
    m_messageContainer = new QWidget;
    m_messageContainer->setObjectName("messageContainer");
    m_messageScrollArea->setWidget(m_messageContainer);
    
    // Message layout
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(0, 16, 0, 16);
    m_messageLayout->setSpacing(8);
    m_messageLayout->addStretch(); // Push messages to bottom initially
    
    m_mainSplitter->addWidget(m_messageScrollArea);
}

void ChatWidget::setupInputArea()
{
    m_inputFrame = new QFrame;
    m_inputFrame->setObjectName("inputFrame");
    m_inputFrame->setFrameStyle(QFrame::NoFrame);
    m_inputFrame->setFixedHeight(120);
    
    m_inputLayout = new QVBoxLayout(m_inputFrame);
    m_inputLayout->setContentsMargins(16, 16, 16, 16);
    m_inputLayout->setSpacing(8);
    
    // Input controls layout
    m_inputControlsLayout = new QHBoxLayout;
    m_inputControlsLayout->setSpacing(8);
    
    // Text input
    m_inputTextEdit = new QTextEdit;
    m_inputTextEdit->setObjectName("inputTextEdit");
    m_inputTextEdit->setMaximumHeight(80);
    m_inputTextEdit->setPlaceholderText("Type your message here... (Press Enter to send, Shift+Enter for new line)");
    connect(m_inputTextEdit, &QTextEdit::textChanged, this, &ChatWidget::onInputTextChanged);
    
    // Buttons
    m_attachButton = new QPushButton("📎");
    m_attachButton->setProperty("class", "icon-button");
    m_attachButton->setToolTip("Attach file");
    m_attachButton->setFixedSize(40, 40);
    connect(m_attachButton, &QPushButton::clicked, this, [this]() {
        QString filename = m_fileManager->openFileDialog("Attach File", "All Files (*.*)");
        if (!filename.isEmpty()) {
            addAttachment(filename);
        }
    });
    
    m_sendButton = new QPushButton("Send");
    m_sendButton->setProperty("class", "primary-button");
    m_sendButton->setEnabled(false);
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWidget::sendMessage);
    
    m_clearButton = new QPushButton("Clear");
    m_clearButton->setProperty("class", "secondary-button");
    connect(m_clearButton, &QPushButton::clicked, this, &ChatWidget::clearAttachments);
    
    // Assemble input controls
    m_inputControlsLayout->addWidget(m_inputTextEdit, 1);
    m_inputControlsLayout->addWidget(m_attachButton);
    m_inputControlsLayout->addWidget(m_sendButton);
    m_inputControlsLayout->addWidget(m_clearButton);
    
    // Status indicators
    QHBoxLayout* statusLayout = new QHBoxLayout;
    statusLayout->setSpacing(16);
    
    m_typingIndicator = new QLabel;
    m_typingIndicator->setProperty("class", "streaming-indicator");
    m_typingIndicator->setVisible(false);
    
    m_streamProgress = new QProgressBar;
    m_streamProgress->setVisible(false);
    m_streamProgress->setFixedHeight(4);
    
    m_tokenCountLabel = new QLabel("Ready");
    m_tokenCountLabel->setProperty("class", "card-subtitle");
    
    statusLayout->addWidget(m_typingIndicator);
    statusLayout->addWidget(m_streamProgress, 1);
    statusLayout->addStretch();
    statusLayout->addWidget(m_tokenCountLabel);
    
    m_inputLayout->addLayout(m_inputControlsLayout);
    m_inputLayout->addLayout(statusLayout);
    
    m_mainSplitter->addWidget(m_inputFrame);
    m_mainSplitter->setSizes({1000, 120});
}

void ChatWidget::setupAttachmentArea()
{
    m_attachmentFrame = new QFrame;
    m_attachmentFrame->setObjectName("attachmentFrame");
    m_attachmentFrame->setFrameStyle(QFrame::NoFrame);
    m_attachmentFrame->setVisible(false);
    
    m_attachmentLayout = new QHBoxLayout(m_attachmentFrame);
    m_attachmentLayout->setContentsMargins(8, 8, 8, 8);
    m_attachmentLayout->setSpacing(8);
    
    // Insert attachment frame between input controls and status
    m_inputLayout->insertWidget(1, m_attachmentFrame);
}

void ChatWidget::addMessage(const Message &message)
{
    m_messages.push_back(message);
    
    // Create message widget
    MessageWidget* messageWidget = createMessageWidget(message);
    m_messageWidgets.push_back(messageWidget);
    
    // Insert before stretch
    int insertIndex = m_messageLayout->count() - 1;
    m_messageLayout->insertWidget(insertIndex, messageWidget);
    
    // Animate and scroll
    animateNewMessage();
    QTimer::singleShot(100, this, &ChatWidget::scrollToBottom);
    
    emit messageAdded(message);
    emit conversationChanged();
}

void ChatWidget::clearHistory()
{
    // Clear messages
    m_messages.clear();
    
    // Clear message widgets
    for (MessageWidget* widget : m_messageWidgets) {
        widget->deleteLater();
    }
    m_messageWidgets.clear();
    
    // Clear attachments
    clearAttachments();
    
    emit conversationChanged();
}

void ChatWidget::loadHistory()
{
    // This would load from a database or file
    // For now, just emit the signal
    emit conversationChanged();
}

void ChatWidget::saveHistory()
{
    // This would save to a database or file
    // For now, just emit the signal
    emit conversationChanged();
}

void ChatWidget::saveConversation(const QString &filename)
{
    if (m_fileManager) {
        m_fileManager->saveConversation(filename, m_messages);
    }
}

void ChatWidget::loadConversation(const QString &filename)
{
    if (m_fileManager) {
        std::vector<Message> messages;
        if (m_fileManager->loadConversation(filename, messages)) {
            clearHistory();
            for (const auto& message : messages) {
                addMessage(message);
            }
        }
    }
}

void ChatWidget::exportMarkdown(const QString &filename)
{
    if (m_fileManager) {
        m_fileManager->exportMarkdown(filename, m_messages);
    }
}

void ChatWidget::focusInput()
{
    m_inputTextEdit->setFocus();
}

bool ChatWidget::isInputFocused() const
{
    return m_inputTextEdit->hasFocus();
}

int ChatWidget::getTotalTokens() const
{
    int total = 0;
    for (const auto& message : m_messages) {
        total += message.totalTokens;
    }
    return total;
}

double ChatWidget::getAverageTokensPerSecond() const
{
    if (m_messages.empty()) return 0.0;
    
    double totalTPS = 0.0;
    int streamingMessages = 0;
    
    for (const auto& message : m_messages) {
        if (message.tokensPerSecond > 0.0) {
            totalTPS += message.tokensPerSecond;
            streamingMessages++;
        }
    }
    
    return streamingMessages > 0 ? totalTPS / streamingMessages : 0.0;
}

void ChatWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ChatWidget::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                addAttachment(url.toLocalFile());
            }
        }
    }
}

void ChatWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (event->modifiers() & Qt::ShiftModifier) {
            // Shift+Enter: new line
            QWidget::keyPressEvent(event);
        } else {
            // Enter: send message
            sendMessage();
            event->accept();
            return;
        }
    }
    
    QWidget::keyPressEvent(event);
}

void ChatWidget::sendMessage()
{
    QString text = m_inputTextEdit->toPlainText().trimmed();
    if (text.isEmpty() && m_pendingAttachments.empty()) {
        return;
    }
    
    // Create user message
    Message userMessage(text, MessageRole::User);
    for (auto attachment : m_pendingAttachments) {
        userMessage.addAttachment(attachment);
    }
    
    addMessage(userMessage);
    
    // Clear input
    m_inputTextEdit->clear();
    clearAttachments();
    
    // Create assistant message for streaming
    m_currentMessage = Message("", MessageRole::Assistant);
    m_currentMessage.startStreaming();
    m_streamingMessage = &m_currentMessage;
    
    // Add placeholder for assistant response
    addMessage(m_currentMessage);
    
    // Update UI state
    m_isStreaming = true;
    m_typingIndicator->setText("AI is thinking...");
    m_typingIndicator->setVisible(true);
    m_streamProgress->setVisible(true);
    m_streamProgress->setRange(0, 0); // Indeterminate
    updateSendButton();
    
    // Send to API
    if (m_api) {
        m_api->sendMessage(m_messages);
    }
}

void ChatWidget::onInputTextChanged()
{
    updateSendButton();
    
    // Show typing indicator briefly
    m_typingTimer->start(500);
}

void ChatWidget::onStreamReceived(const QString &content)
{
    if (!m_isStreaming || !m_streamingMessage) return;
    
    // Update the streaming message
    m_streamingMessage->content += content;
    m_streamingMessage->updateStreaming(m_streamingMessage->content);
    
    // Update the message widget
    if (!m_messageWidgets.empty()) {
        MessageWidget* lastWidget = m_messageWidgets.back();
        if (lastWidget && lastWidget->getRole() == MessageRole::Assistant) {
            lastWidget->updateMessage(*m_streamingMessage);
        }
    }
    
    // Auto-scroll if needed
    if (m_autoScroll) {
        QTimer::singleShot(10, this, &ChatWidget::scrollToBottom);
    }
    
    emit tokenStatsChanged(getTotalTokens(), getAverageTokensPerSecond());
}

void ChatWidget::onStreamCompleted(bool success)
{
    if (!m_isStreaming) return;
    
    m_isStreaming = false;
    m_typingIndicator->setVisible(false);
    m_streamProgress->setVisible(false);
    updateSendButton();
    
    if (m_streamingMessage) {
        if (success) {
            m_streamingMessage->completeStreaming();
        } else {
            m_streamingMessage->setError();
        }
        
        // Update the message widget one final time
        if (!m_messageWidgets.empty()) {
            MessageWidget* lastWidget = m_messageWidgets.back();
            if (lastWidget && lastWidget->getRole() == MessageRole::Assistant) {
                lastWidget->updateMessage(*m_streamingMessage);
            }
        }
        
        m_streamingMessage = nullptr;
    }
    
    emit conversationChanged();
    emit tokenStatsChanged(getTotalTokens(), getAverageTokensPerSecond());
}

void ChatWidget::onStreamError(const QString &error)
{
    m_isStreaming = false;
    m_typingIndicator->setText(QString("Error: %1").arg(error));
    m_streamProgress->setVisible(false);
    updateSendButton();
    
    if (m_streamingMessage) {
        m_streamingMessage->setError();
        m_streamingMessage = nullptr;
    }
    
    // Hide error after a few seconds
    QTimer::singleShot(5000, [this]() {
        m_typingIndicator->setVisible(false);
    });
}

void ChatWidget::scrollToBottom()
{
    QScrollBar* scrollBar = m_messageScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ChatWidget::updateTypingIndicator()
{
    if (!m_isStreaming) {
        m_typingIndicator->setVisible(false);
    }
}

void ChatWidget::clearAttachments()
{
    m_pendingAttachments.clear();
    
    // Clear attachment labels
    for (QLabel* label : m_attachmentLabels) {
        label->deleteLater();
    }
    m_attachmentLabels.clear();
    
    m_attachmentFrame->setVisible(false);
    updateSendButton();
}

void ChatWidget::addAttachment(const QString &filePath)
{
    if (m_fileManager) {
        auto attachment = m_fileManager->createAttachment(filePath);
        if (attachment) {
            m_pendingAttachments.push_back(attachment);
            
            // Create attachment preview
            QLabel* label = new QLabel(QFileInfo(filePath).fileName());
            label->setProperty("class", "attachment-preview");
            m_attachmentLabels.push_back(label);
            m_attachmentLayout->addWidget(label);
            
            m_attachmentFrame->setVisible(true);
            updateSendButton();
        }
    }
}

void ChatWidget::removeAttachment(int index)
{
    if (index >= 0 && index < static_cast<int>(m_pendingAttachments.size())) {
        m_pendingAttachments.erase(m_pendingAttachments.begin() + index);
        
        if (index < static_cast<int>(m_attachmentLabels.size())) {
            m_attachmentLabels[index]->deleteLater();
            m_attachmentLabels.erase(m_attachmentLabels.begin() + index);
        }
        
        if (m_pendingAttachments.empty()) {
            m_attachmentFrame->setVisible(false);
        }
        
        updateSendButton();
    }
}

void ChatWidget::updateSendButton()
{
    bool hasText = !m_inputTextEdit->toPlainText().trimmed().isEmpty();
    bool hasAttachments = !m_pendingAttachments.empty();
    bool canSend = (hasText || hasAttachments) && !m_isStreaming;
    
    m_sendButton->setEnabled(canSend);
}

void ChatWidget::animateNewMessage()
{
    // Simple animation for new messages
    // This could be enhanced with more sophisticated animations
    if (!m_messageWidgets.empty()) {
        MessageWidget* lastWidget = m_messageWidgets.back();
        if (lastWidget) {
            lastWidget->startFadeInAnimation();
        }
    }
}

void ChatWidget::updateTokenStats()
{
    if (m_isStreaming && m_streamingMessage) {
        QString stats = QString("Tokens: %1 | TPS: %2")
            .arg(m_streamingMessage->totalTokens)
            .arg(m_streamingMessage->tokensPerSecond, 0, 'f', 1);
        m_tokenCountLabel->setText(stats);
    } else {
        int totalTokens = getTotalTokens();
        if (totalTokens > 0) {
            m_tokenCountLabel->setText(QString("Total tokens: %1").arg(totalTokens));
        } else {
            m_tokenCountLabel->setText("Ready");
        }
    }
}

MessageWidget* ChatWidget::createMessageWidget(const Message &message)
{
    return new MessageWidget(message, m_markdownRenderer.get(), this);
}

void ChatWidget::updateMessageWidget(MessageWidget *widget, const Message &message)
{
    if (widget) {
        widget->updateMessage(message);
    }
}

void ChatWidget::HandleFileDrops(const std::vector<QString>& filePaths)
{
    for (const QString& filePath : filePaths) {
        addAttachment(filePath);
    }
} 