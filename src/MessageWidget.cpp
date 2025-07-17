#include "MessageWidget.h"
#include "MarkdownRenderer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPixmap>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

MessageWidget::MessageWidget(const Message& message, QWidget* parent)
    : QWidget(parent)
    , m_message(message)
    , m_avatarLabel(nullptr)
    , m_nameLabel(nullptr)
    , m_timestampLabel(nullptr)
    , m_contentLabel(nullptr)
    , m_attachmentsWidget(nullptr)
    , m_markdownRenderer(new MarkdownRenderer(this))
{
    setupUI();
    updateContent();
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

void MessageWidget::setupUI()
{
    setObjectName("MessageWidget");
    
    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(6);
    
    // Message container with card styling
    auto* messageFrame = new QFrame;
    messageFrame->setObjectName("MessageFrame");
    messageFrame->setFrameShape(QFrame::NoFrame);
    
    auto* frameLayout = new QVBoxLayout(messageFrame);
    frameLayout->setContentsMargins(16, 12, 16, 12);
    frameLayout->setSpacing(8);
    
    // Header layout (avatar, name, timestamp)
    auto* headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(8);
    
    // Avatar
    m_avatarLabel = new QLabel;
    m_avatarLabel->setFixedSize(32, 32);
    m_avatarLabel->setObjectName("AvatarLabel");
    updateAvatar();
    
    // Name and timestamp container
    auto* nameTimeLayout = new QVBoxLayout;
    nameTimeLayout->setSpacing(2);
    nameTimeLayout->setContentsMargins(0, 0, 0, 0);
    
    m_nameLabel = new QLabel;
    m_nameLabel->setObjectName("NameLabel");
    m_nameLabel->setStyleSheet("font-weight: 600; font-size: 14px;");
    
    m_timestampLabel = new QLabel;
    m_timestampLabel->setObjectName("TimestampLabel");
    m_timestampLabel->setStyleSheet("color: #6B7280; font-size: 12px;");
    
    nameTimeLayout->addWidget(m_nameLabel);
    nameTimeLayout->addWidget(m_timestampLabel);
    
    headerLayout->addWidget(m_avatarLabel);
    headerLayout->addLayout(nameTimeLayout);
    headerLayout->addStretch();
    
    // Content
    m_contentLabel = new QLabel;
    m_contentLabel->setObjectName("ContentLabel");
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksClickableByMouse);
    m_contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_contentLabel->setStyleSheet("font-size: 14px; line-height: 1.5;");
    
    // Attachments widget
    m_attachmentsWidget = new QWidget;
    m_attachmentsWidget->setObjectName("AttachmentsWidget");
    auto* attachmentsLayout = new QVBoxLayout(m_attachmentsWidget);
    attachmentsLayout->setContentsMargins(0, 0, 0, 0);
    attachmentsLayout->setSpacing(4);
    
    frameLayout->addLayout(headerLayout);
    frameLayout->addWidget(m_contentLabel);
    frameLayout->addWidget(m_attachmentsWidget);
    
    mainLayout->addWidget(messageFrame);
    
    // Apply styling based on message role
    updateStyling();
    
    // Add subtle shadow effect
    auto* shadowEffect = new QGraphicsDropShadowEffect;
    shadowEffect->setBlurRadius(8);
    shadowEffect->setColor(QColor(0, 0, 0, 15));
    shadowEffect->setOffset(0, 1);
    messageFrame->setGraphicsEffect(shadowEffect);
}

void MessageWidget::updateContent()
{
    // Update name
    QString displayName = m_message.role();
    if (displayName == "user") {
        displayName = "You";
    } else if (displayName == "assistant") {
        displayName = "Assistant";
    } else if (displayName == "system") {
        displayName = "System";
    }
    m_nameLabel->setText(displayName);
    
    // Update timestamp
    m_timestampLabel->setText(m_message.timestamp().toString("hh:mm AP"));
    
    // Update content with markdown rendering
    if (!m_message.content().isEmpty()) {
        QString renderedContent = m_markdownRenderer->renderMarkdown(m_message.content());
        m_contentLabel->setText(renderedContent);
        m_contentLabel->setVisible(true);
    } else {
        m_contentLabel->setVisible(false);
    }
    
    // Update attachments
    updateAttachments();
}

void MessageWidget::updateAttachments()
{
    // Clear existing attachments
    QLayoutItem* item;
    while ((item = m_attachmentsWidget->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    const auto& attachments = m_message.attachments();
    if (attachments.isEmpty()) {
        m_attachmentsWidget->setVisible(false);
        return;
    }
    
    m_attachmentsWidget->setVisible(true);
    
    for (const auto& attachment : attachments) {
        auto* attachmentWidget = createAttachmentWidget(attachment);
        m_attachmentsWidget->layout()->addWidget(attachmentWidget);
    }
}

QWidget* MessageWidget::createAttachmentWidget(const MessageAttachment& attachment)
{
    auto* widget = new QFrame;
    widget->setObjectName("AttachmentWidget");
    widget->setFrameShape(QFrame::Box);
    widget->setFrameStyle(QFrame::Raised);
    widget->setStyleSheet(
        "QFrame#AttachmentWidget {"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "    background-color: #F9FAFB;"
        "    padding: 8px;"
        "}"
        "QFrame#AttachmentWidget:hover {"
        "    background-color: #F3F4F6;"
        "    border-color: #D1D5DB;"
        "}"
    );
    
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    
    // File icon/preview
    auto* iconLabel = new QLabel;
    iconLabel->setFixedSize(32, 32);
    
    if (attachment.type() == "image") {
        // Show image preview
        QPixmap pixmap;
        if (pixmap.loadFromData(attachment.data())) {
            iconLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            iconLabel->setText("🖼️");
            iconLabel->setAlignment(Qt::AlignCenter);
        }
    } else {
        // Show file icon
        iconLabel->setText("📄");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("font-size: 16px;");
    }
    
    // File info
    auto* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    
    auto* nameLabel = new QLabel(attachment.filename());
    nameLabel->setStyleSheet("font-weight: 500; font-size: 13px;");
    
    auto* sizeLabel = new QLabel(formatFileSize(attachment.data().size()));
    sizeLabel->setStyleSheet("color: #6B7280; font-size: 11px;");
    
    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(sizeLabel);
    
    layout->addWidget(iconLabel);
    layout->addLayout(infoLayout);
    layout->addStretch();
    
    // Make clickable for image preview
    if (attachment.type() == "image") {
        widget->setCursor(Qt::PointingHandCursor);
        widget->installEventFilter(this);
        widget->setProperty("attachment_data", attachment.data());
    }
    
    return widget;
}

void MessageWidget::updateAvatar()
{
    QString role = m_message.role();
    QPixmap avatar(32, 32);
    avatar.fill(Qt::transparent);
    
    QPainter painter(&avatar);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Create circular avatar with role-based color
    QColor avatarColor;
    QString avatarText;
    
    if (role == "user") {
        avatarColor = QColor("#3B82F6");  // Blue
        avatarText = "U";
    } else if (role == "assistant") {
        avatarColor = QColor("#10B981");  // Green
        avatarText = "A";
    } else {
        avatarColor = QColor("#6B7280");  // Gray
        avatarText = "S";
    }
    
    painter.setBrush(avatarColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 32, 32);
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRect(0, 0, 32, 32), Qt::AlignCenter, avatarText);
    
    m_avatarLabel->setPixmap(avatar);
}

void MessageWidget::updateStyling()
{
    QString role = m_message.role();
    
    if (role == "user") {
        setStyleSheet(
            "QFrame#MessageFrame {"
            "    background-color: #EFF6FF;"
            "    border: 1px solid #DBEAFE;"
            "    border-radius: 12px;"
            "}"
        );
    } else if (role == "assistant") {
        setStyleSheet(
            "QFrame#MessageFrame {"
            "    background-color: #F0FDF4;"
            "    border: 1px solid #DCFCE7;"
            "    border-radius: 12px;"
            "}"
        );
    } else {
        setStyleSheet(
            "QFrame#MessageFrame {"
            "    background-color: #F9FAFB;"
            "    border: 1px solid #E5E7EB;"
            "    border-radius: 12px;"
            "}"
        );
    }
}

void MessageWidget::setMessage(const Message& message)
{
    m_message = message;
    updateContent();
    updateAvatar();
    updateStyling();
}

Message MessageWidget::message() const
{
    return m_message;
}

void MessageWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    
    auto* copyAction = menu.addAction("Copy Message");
    copyAction->setIcon(QIcon("📋"));
    connect(copyAction, &QAction::triggered, this, &MessageWidget::copyMessage);
    
    if (!m_message.attachments().isEmpty()) {
        menu.addSeparator();
        auto* saveAttachmentsAction = menu.addAction("Save Attachments...");
        saveAttachmentsAction->setIcon(QIcon("💾"));
        connect(saveAttachmentsAction, &QAction::triggered, this, &MessageWidget::saveAttachments);
    }
    
    menu.exec(event->globalPos());
}

bool MessageWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget* widget = qobject_cast<QWidget*>(watched);
            if (widget && widget->property("attachment_data").isValid()) {
                showImagePreview(widget->property("attachment_data").toByteArray());
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MessageWidget::copyMessage()
{
    QApplication::clipboard()->setText(m_message.content());
}

void MessageWidget::saveAttachments()
{
    // TODO: Implement file save dialog for attachments
    // This would typically use QFileDialog to let user choose save location
}

void MessageWidget::showImagePreview(const QByteArray& imageData)
{
    // TODO: Implement image preview dialog
    // This would create a popup dialog showing the full-size image
}

QString MessageWidget::formatFileSize(qint64 bytes)
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

void MessageWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MessageWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void MessageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void MessageWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
}

void MessageWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
} 