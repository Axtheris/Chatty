#include "WelcomeWidget.h"
#include "Settings.h"
#include "OpenRouterAPI.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QDesktopServices>

WelcomeWidget::WelcomeWidget(Settings* settings, OpenRouterAPI* api, QWidget *parent)
    : QWidget(parent)
    , m_settings(settings)
    , m_api(api)
{
    setAcceptDrops(true);
    setupUI();
    applyTheme();
    
    // Setup animation timer
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &WelcomeWidget::animateCards);
    
    // Refresh content
    updateGreeting();
    updateStats();
    refreshRecentFiles();
}

WelcomeWidget::~WelcomeWidget() = default;

void WelcomeWidget::setupUI()
{
    // Main scroll area
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarNever);
    
    m_scrollWidget = new QWidget;
    m_scrollArea->setWidget(m_scrollWidget);
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);
    
    m_mainLayout = new QVBoxLayout(m_scrollWidget);
    m_mainLayout->setContentsMargins(32, 32, 32, 32);
    m_mainLayout->setSpacing(32);
    
    // Setup sections
    setupHeader();
    setupQuickActions();
    setupTemplates();
    setupRecentFiles();
    setupStats();
    
    m_mainLayout->addStretch();
}

void WelcomeWidget::setupHeader()
{
    m_headerFrame = new QFrame;
    m_headerFrame->setFrameStyle(QFrame::NoFrame);
    
    m_headerLayout = new QVBoxLayout(m_headerFrame);
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(8);
    
    // Greeting
    m_greetingLabel = new QLabel;
    m_greetingLabel->setProperty("class", "greeting-title");
    m_greetingLabel->setAlignment(Qt::AlignLeft);
    
    // Subtitle
    m_subtitleLabel = new QLabel("How can I help you today?");
    m_subtitleLabel->setProperty("class", "greeting-subtitle");
    m_subtitleLabel->setAlignment(Qt::AlignLeft);
    
    // Time label
    m_timeLabel = new QLabel;
    m_timeLabel->setProperty("class", "greeting-subtitle");
    m_timeLabel->setAlignment(Qt::AlignLeft);
    
    m_headerLayout->addWidget(m_greetingLabel);
    m_headerLayout->addWidget(m_subtitleLabel);
    m_headerLayout->addWidget(m_timeLabel);
    
    m_mainLayout->addWidget(m_headerFrame);
}

void WelcomeWidget::setupQuickActions()
{
    m_quickActionsFrame = new QFrame;
    m_quickActionsFrame->setFrameStyle(QFrame::NoFrame);
    
    m_quickActionsLayout = new QVBoxLayout(m_quickActionsFrame);
    m_quickActionsLayout->setContentsMargins(0, 0, 0, 0);
    m_quickActionsLayout->setSpacing(16);
    
    // Section title
    m_quickActionsTitleLabel = new QLabel("Quick Actions");
    m_quickActionsTitleLabel->setProperty("class", "section-title");
    m_quickActionsLayout->addWidget(m_quickActionsTitleLabel);
    
    // Actions layout
    m_actionsLayout = new QHBoxLayout;
    m_actionsLayout->setSpacing(16);
    
    // Create action buttons
    m_newChatButton = new QPushButton("🆕 New Chat");
    m_newChatButton->setProperty("class", "welcome-action");
    connect(m_newChatButton, &QPushButton::clicked, this, &WelcomeWidget::onNewChatClicked);
    
    m_uploadFileButton = new QPushButton("📎 Upload File");
    m_uploadFileButton->setProperty("class", "welcome-action secondary");
    connect(m_uploadFileButton, &QPushButton::clicked, this, [this]() {
        // This will be handled by file dialog
        emit fileDropped(QString()); // Empty string triggers file dialog
    });
    
    m_settingsButton = new QPushButton("⚙️ Settings");
    m_settingsButton->setProperty("class", "welcome-action secondary");
    
    m_actionsLayout->addWidget(m_newChatButton);
    m_actionsLayout->addWidget(m_uploadFileButton);
    m_actionsLayout->addWidget(m_settingsButton);
    m_actionsLayout->addStretch();
    
    m_quickActionsLayout->addLayout(m_actionsLayout);
    m_mainLayout->addWidget(m_quickActionsFrame);
}

void WelcomeWidget::setupTemplates()
{
    m_templatesFrame = new QFrame;
    m_templatesFrame->setFrameStyle(QFrame::NoFrame);
    
    m_templatesLayout = new QVBoxLayout(m_templatesFrame);
    m_templatesLayout->setContentsMargins(0, 0, 0, 0);
    m_templatesLayout->setSpacing(16);
    
    // Section title
    m_templatesTitleLabel = new QLabel("Chat Templates");
    m_templatesTitleLabel->setProperty("class", "section-title");
    m_templatesLayout->addWidget(m_templatesTitleLabel);
    
    // Templates grid
    m_templatesGrid = new QGridLayout;
    m_templatesGrid->setSpacing(16);
    
    // Create template cards
    int row = 0, col = 0;
    for (const auto& template_ : m_templates) {
        QFrame* card = createTemplateCard(template_.title, template_.description, template_.prompt);
        m_templatesGrid->addWidget(card, row, col);
        
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }
    
    m_templatesLayout->addLayout(m_templatesGrid);
    m_mainLayout->addWidget(m_templatesFrame);
}

void WelcomeWidget::setupRecentFiles()
{
    m_recentFilesFrame = new QFrame;
    m_recentFilesFrame->setFrameStyle(QFrame::NoFrame);
    
    m_recentFilesLayout = new QVBoxLayout(m_recentFilesFrame);
    m_recentFilesLayout->setContentsMargins(0, 0, 0, 0);
    m_recentFilesLayout->setSpacing(16);
    
    // Section title
    m_recentFilesTitleLabel = new QLabel("Recent Conversations");
    m_recentFilesTitleLabel->setProperty("class", "section-title");
    m_recentFilesLayout->addWidget(m_recentFilesTitleLabel);
    
    // Recent files grid
    m_recentFilesGrid = new QGridLayout;
    m_recentFilesGrid->setSpacing(12);
    
    // Will be populated by refreshRecentFiles()
    
    m_recentFilesLayout->addLayout(m_recentFilesGrid);
    m_mainLayout->addWidget(m_recentFilesFrame);
}

void WelcomeWidget::setupStats()
{
    m_statsFrame = new QFrame;
    m_statsFrame->setFrameStyle(QFrame::NoFrame);
    
    m_statsLayout = new QVBoxLayout(m_statsFrame);
    m_statsLayout->setContentsMargins(0, 0, 0, 0);
    m_statsLayout->setSpacing(16);
    
    // Section title
    m_statsTitleLabel = new QLabel("Usage Statistics");
    m_statsTitleLabel->setProperty("class", "section-title");
    m_statsLayout->addWidget(m_statsTitleLabel);
    
    // Stats cards layout
    m_statsCardsLayout = new QHBoxLayout;
    m_statsCardsLayout->setSpacing(16);
    
    // Create stats cards
    QFrame* totalChatsCard = createStatsCard("Total Chats", "0", "");
    QFrame* totalTokensCard = createStatsCard("Tokens Used", "0", "");
    QFrame* avgResponseCard = createStatsCard("Avg Response", "0s", "");
    
    m_statsCardsLayout->addWidget(totalChatsCard);
    m_statsCardsLayout->addWidget(totalTokensCard);
    m_statsCardsLayout->addWidget(avgResponseCard);
    m_statsCardsLayout->addStretch();
    
    m_statsLayout->addLayout(m_statsCardsLayout);
    m_mainLayout->addWidget(m_statsFrame);
}

QFrame* WelcomeWidget::createCard(const QString& title, const QString& subtitle, const QString& iconPath)
{
    QFrame* card = new QFrame;
    card->setProperty("class", "welcome-card");
    card->setFrameStyle(QFrame::NoFrame);
    
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);
    
    // Title
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setProperty("class", "card-title");
    layout->addWidget(titleLabel);
    
    // Subtitle
    if (!subtitle.isEmpty()) {
        QLabel* subtitleLabel = new QLabel(subtitle);
        subtitleLabel->setProperty("class", "card-subtitle");
        layout->addWidget(subtitleLabel);
    }
    
    layout->addStretch();
    
    return card;
}

QFrame* WelcomeWidget::createStatsCard(const QString& title, const QString& value, const QString& trend)
{
    QFrame* card = new QFrame;
    card->setProperty("class", "stats-card");
    card->setFrameStyle(QFrame::NoFrame);
    
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(4);
    layout->setAlignment(Qt::AlignCenter);
    
    // Value
    QLabel* valueLabel = new QLabel(value);
    valueLabel->setProperty("class", "stats-value");
    valueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(valueLabel);
    
    // Title
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setProperty("class", "stats-label");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // Trend (if provided)
    if (!trend.isEmpty()) {
        QLabel* trendLabel = new QLabel(trend);
        trendLabel->setProperty("class", "stats-label");
        trendLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(trendLabel);
    }
    
    return card;
}

QFrame* WelcomeWidget::createTemplateCard(const QString& title, const QString& description, const QString& template_)
{
    QFrame* card = new QFrame;
    card->setProperty("class", "template-card");
    card->setFrameStyle(QFrame::NoFrame);
    card->setCursor(Qt::PointingHandCursor);
    
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);
    
    // Title
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setProperty("class", "card-title");
    layout->addWidget(titleLabel);
    
    // Description
    QLabel* descLabel = new QLabel(description);
    descLabel->setProperty("class", "card-description");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);
    
    layout->addStretch();
    
    // Connect click event
    card->mousePressEvent = [this, template_](QMouseEvent* event) {
        Q_UNUSED(event)
        emit templateSelected(template_);
    };
    
    return card;
}

QFrame* WelcomeWidget::createRecentFileCard(const QString& filename, const QString& path, const QDateTime& modified)
{
    QFrame* card = new QFrame;
    card->setProperty("class", "recent-file-card");
    card->setFrameStyle(QFrame::NoFrame);
    card->setCursor(Qt::PointingHandCursor);
    
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);
    
    // Filename
    QLabel* nameLabel = new QLabel(filename);
    nameLabel->setProperty("class", "card-title");
    nameLabel->setElideMode(Qt::ElideRight);
    layout->addWidget(nameLabel);
    
    // Modified time
    QLabel* timeLabel = new QLabel(modified.toString("MMM dd, hh:mm"));
    timeLabel->setProperty("class", "card-subtitle");
    layout->addWidget(timeLabel);
    
    // Connect click event
    card->mousePressEvent = [this, path](QMouseEvent* event) {
        Q_UNUSED(event)
        emit recentFileOpened(path);
    };
    
    return card;
}

void WelcomeWidget::updateGreeting()
{
    QTime currentTime = QTime::currentTime();
    QString greeting;
    
    if (currentTime.hour() < 12) {
        greeting = "Good morning! 🌅";
    } else if (currentTime.hour() < 17) {
        greeting = "Good afternoon! ☀️";
    } else {
        greeting = "Good evening! 🌙";
    }
    
    // Get user name
    QString userName = qgetenv("USER");
    if (userName.isEmpty()) {
        userName = qgetenv("USERNAME");
    }
    if (!userName.isEmpty()) {
        greeting = QString("Welcome, %1! 👋").arg(userName);
    }
    
    m_greetingLabel->setText(greeting);
    
    // Update time
    m_timeLabel->setText(QDateTime::currentDateTime().toString("dddd, MMMM dd, yyyy"));
}

void WelcomeWidget::updateStats()
{
    // This would typically load from settings or a database
    // For now, show placeholder values
    
    // Find stats cards and update them
    QList<QFrame*> statsCards = m_statsFrame->findChildren<QFrame*>();
    for (QFrame* card : statsCards) {
        if (card->property("class").toString() == "stats-card") {
            // Update logic would go here
        }
    }
}

void WelcomeWidget::refreshRecentFiles()
{
    // Clear existing items
    QLayoutItem* item;
    while ((item = m_recentFilesGrid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Get conversations directory
    QString conversationsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/conversations";
    QDir dir(conversationsPath);
    
    if (dir.exists()) {
        QStringList filters;
        filters << "*.json";
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);
        
        int row = 0, col = 0;
        int maxFiles = 6; // Show up to 6 recent files
        
        for (int i = 0; i < qMin(files.size(), maxFiles); ++i) {
            const QFileInfo& fileInfo = files[i];
            QFrame* card = createRecentFileCard(
                fileInfo.baseName(),
                fileInfo.absoluteFilePath(),
                fileInfo.lastModified()
            );
            
            m_recentFilesGrid->addWidget(card, row, col);
            
            col++;
            if (col >= 3) {
                col = 0;
                row++;
            }
        }
    }
    
    // If no recent files, show a placeholder
    if (m_recentFilesGrid->count() == 0) {
        QLabel* placeholder = new QLabel("No recent conversations");
        placeholder->setProperty("class", "card-subtitle");
        placeholder->setAlignment(Qt::AlignCenter);
        m_recentFilesGrid->addWidget(placeholder, 0, 0, 1, 3);
    }
}

void WelcomeWidget::applyTheme()
{
    // Apply the modern stylesheet to this widget
    // The parent window will handle the main stylesheet loading
}

void WelcomeWidget::startAnimations()
{
    if (!m_isVisible) return;
    
    m_animationDelay = 0;
    m_animationTimer->start(100); // Start animations with 100ms intervals
}

void WelcomeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void WelcomeWidget::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                emit fileDropped(url.toLocalFile());
                break; // Handle first file only
            }
        }
    }
}

void WelcomeWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_isVisible = true;
    
    // Update dynamic content
    updateGreeting();
    updateStats();
    refreshRecentFiles();
    
    // Start animations after a short delay
    QTimer::singleShot(200, this, &WelcomeWidget::startAnimations);
}

void WelcomeWidget::onNewChatClicked()
{
    emit newChatRequested();
}

void WelcomeWidget::onTemplateClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        // Get template from button data
        QString template_ = button->property("template").toString();
        emit templateSelected(template_);
    }
}

void WelcomeWidget::onRecentFileClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString filePath = button->property("filePath").toString();
        emit recentFileOpened(filePath);
    }
}

void WelcomeWidget::animateCards()
{
    // Simple fade-in animation for cards
    // This could be enhanced with more sophisticated animations
    
    static int cardIndex = 0;
    QList<QFrame*> cards;
    
    // Collect all cards
    cards.append(m_headerFrame->findChildren<QFrame*>());
    cards.append(m_quickActionsFrame->findChildren<QFrame*>());
    cards.append(m_templatesFrame->findChildren<QFrame*>());
    cards.append(m_recentFilesFrame->findChildren<QFrame*>());
    cards.append(m_statsFrame->findChildren<QFrame*>());
    
    if (cardIndex < cards.size()) {
        QFrame* card = cards[cardIndex];
        if (card) {
            // Simple opacity animation
            QPropertyAnimation* animation = new QPropertyAnimation(card, "windowOpacity");
            animation->setDuration(300);
            animation->setStartValue(0.0);
            animation->setEndValue(1.0);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        cardIndex++;
    } else {
        m_animationTimer->stop();
        cardIndex = 0; // Reset for next time
    }
} 