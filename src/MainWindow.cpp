#include "MainWindow.h"
#include "ChatWidget.h"
#include "WelcomeWidget.h"
#include "OpenRouterAPI.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "FileManager.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QAction>
#include <QTimer>
#include <QFrame>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_darkMode(true)
{
    setWindowTitle("Chatty - AI Chat Assistant");
    setMinimumSize(1000, 600);
    resize(1280, 720);
    setAcceptDrops(true);
    
    // Initialize core components
    m_settings = std::make_unique<Settings>(this);
    m_api = std::make_unique<OpenRouterAPI>(this);
    m_fileManager = std::make_unique<FileManager>(this);
    
    // Setup UI
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupSidebar();
    setupActions();
    setupConnections();
    
    // Load settings and apply theme
    loadSettings();
    applyTheme();
    
    // Setup timers
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    m_statusTimer->start(1000); // Update every second
    
    // Initial status
    updateStatusBar();
    updateUserProfile();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    // Main horizontal layout (sidebar + content)
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainSplitter->setChildrenCollapsible(false);
    m_mainLayout->addWidget(m_mainSplitter);
    
    // Initialize chat and welcome widgets (will be added to splitter in setupSidebar)
    m_chatWidget = std::make_unique<ChatWidget>(m_api.get(), m_fileManager.get());
    m_welcomeWidget = std::make_unique<WelcomeWidget>(m_settings.get(), m_api.get());
    
    // Start with welcome screen
    m_mainSplitter->addWidget(m_welcomeWidget.get());
}

void MainWindow::setupSidebar()
{
    // Create sidebar frame
    m_sidebarFrame = new QFrame;
    m_sidebarFrame->setObjectName("sidebarFrame");
    m_sidebarFrame->setFixedWidth(280);
    m_sidebarFrame->setFrameStyle(QFrame::NoFrame);
    
    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    m_sidebarLayout->setContentsMargins(0, 0, 0, 0);
    m_sidebarLayout->setSpacing(0);
    
    // User profile section
    setupUserProfile();
    
    // Navigation section
    setupNavigation();
    
    // Add sidebar to splitter (before content)
    m_mainSplitter->insertWidget(0, m_sidebarFrame);
    m_mainSplitter->setSizes({280, 1000});
}

void MainWindow::setupUserProfile()
{
    m_userProfileFrame = new QFrame;
    m_userProfileFrame->setObjectName("userProfileFrame");
    m_userProfileFrame->setFrameStyle(QFrame::NoFrame);
    
    m_userProfileLayout = new QVBoxLayout(m_userProfileFrame);
    m_userProfileLayout->setContentsMargins(20, 20, 20, 20);
    m_userProfileLayout->setSpacing(8);
    
    // User avatar
    m_userAvatarLabel = new QLabel;
    m_userAvatarLabel->setFixedSize(48, 48);
    m_userAvatarLabel->setAlignment(Qt::AlignCenter);
    m_userAvatarLabel->setStyleSheet(
        "QLabel { "
        "background-color: #3182ce; "
        "border-radius: 24px; "
        "color: white; "
        "font-size: 18px; "
        "font-weight: bold; "
        "}"
    );
    
    // User name
    m_userNameLabel = new QLabel("User");
    m_userNameLabel->setObjectName("userNameLabel");
    m_userNameLabel->setStyleSheet(
        "QLabel { "
        "font-size: 18px; "
        "font-weight: 600; "
        "color: #1a202c; "
        "}"
    );
    
    // User status
    m_userStatusLabel = new QLabel("Ready to chat");
    m_userStatusLabel->setObjectName("userStatusLabel");
    m_userStatusLabel->setStyleSheet(
        "QLabel { "
        "font-size: 14px; "
        "color: #718096; "
        "}"
    );
    
    // Layout user profile
    QHBoxLayout* avatarLayout = new QHBoxLayout;
    avatarLayout->addWidget(m_userAvatarLabel);
    avatarLayout->addSpacing(12);
    
    QVBoxLayout* userInfoLayout = new QVBoxLayout;
    userInfoLayout->setSpacing(2);
    userInfoLayout->addWidget(m_userNameLabel);
    userInfoLayout->addWidget(m_userStatusLabel);
    
    avatarLayout->addLayout(userInfoLayout);
    avatarLayout->addStretch();
    
    m_userProfileLayout->addLayout(avatarLayout);
    m_sidebarLayout->addWidget(m_userProfileFrame);
}

void MainWindow::setupNavigation()
{
    m_navigationWidget = new QWidget;
    m_navigationLayout = new QVBoxLayout(m_navigationWidget);
    m_navigationLayout->setContentsMargins(8, 16, 8, 16);
    m_navigationLayout->setSpacing(4);
    
    // Create navigation buttons
    m_newChatButton = new QPushButton("🆕 New Chat");
    m_newChatButton->setProperty("class", "nav-button");
    m_newChatButton->setCheckable(true);
    m_newChatButton->setChecked(true);
    
    m_historyButton = new QPushButton("📚 History");
    m_historyButton->setProperty("class", "nav-button");
    m_historyButton->setCheckable(true);
    
    m_savedChatsButton = new QPushButton("💾 Saved Chats");
    m_savedChatsButton->setProperty("class", "nav-button");
    m_savedChatsButton->setCheckable(true);
    
    m_settingsButton = new QPushButton("⚙️ Settings");
    m_settingsButton->setProperty("class", "nav-button");
    m_settingsButton->setCheckable(true);
    
    // Add buttons to layout
    m_navigationLayout->addWidget(m_newChatButton);
    m_navigationLayout->addWidget(m_historyButton);
    m_navigationLayout->addWidget(m_savedChatsButton);
    m_navigationLayout->addStretch();
    m_navigationLayout->addWidget(m_settingsButton);
    
    m_sidebarLayout->addWidget(m_navigationWidget);
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    
    m_newChatAction = new QAction("&New Chat", this);
    m_newChatAction->setShortcut(QKeySequence::New);
    fileMenu->addAction(m_newChatAction);
    
    fileMenu->addSeparator();
    
    m_openChatAction = new QAction("&Open Chat...", this);
    m_openChatAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(m_openChatAction);
    
    m_saveChatAction = new QAction("&Save Chat", this);
    m_saveChatAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction(m_saveChatAction);
    
    m_saveChatAsAction = new QAction("Save Chat &As...", this);
    m_saveChatAsAction->setShortcut(QKeySequence::SaveAs);
    fileMenu->addAction(m_saveChatAsAction);
    
    fileMenu->addSeparator();
    
    m_exportMarkdownAction = new QAction("Export as &Markdown...", this);
    fileMenu->addAction(m_exportMarkdownAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(m_exitAction);
    
    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Copy", this, &MainWindow::copy, QKeySequence::Copy);
    editMenu->addAction("&Paste", this, &MainWindow::paste, QKeySequence::Paste);
    editMenu->addAction("Select &All", this, &MainWindow::selectAll, QKeySequence::SelectAll);
    
    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    
    m_toggleThemeAction = new QAction("Toggle &Theme", this);
    m_toggleThemeAction->setShortcut(QKeySequence("Ctrl+T"));
    viewMenu->addAction(m_toggleThemeAction);
    
    // Tools menu
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");
    
    m_settingsAction = new QAction("&Settings...", this);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    toolsMenu->addAction(m_settingsAction);
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    
    m_aboutAction = new QAction("&About Chatty", this);
    helpMenu->addAction(m_aboutAction);
    
    helpMenu->addAction("About &Qt", qApp, &QApplication::aboutQt);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color: #718096; font-size: 12px;");
    statusBar()->addWidget(m_statusLabel);
    
    statusBar()->addPermanentWidget(new QLabel(""), 1); // Spacer
    
    m_modelLabel = new QLabel("No model selected");
    m_modelLabel->setStyleSheet("color: #4a5568; font-size: 12px;");
    statusBar()->addPermanentWidget(m_modelLabel);
    
    m_tokenStatsLabel = new QLabel("Tokens: 0");
    m_tokenStatsLabel->setStyleSheet("color: #4a5568; font-size: 12px;");
    statusBar()->addPermanentWidget(m_tokenStatsLabel);
    
    m_connectionProgress = new QProgressBar;
    m_connectionProgress->setVisible(false);
    m_connectionProgress->setFixedWidth(100);
    statusBar()->addPermanentWidget(m_connectionProgress);
}

void MainWindow::setupActions()
{
    // Connect menu actions
    connect(m_newChatAction, &QAction::triggered, this, &MainWindow::newChat);
    connect(m_openChatAction, &QAction::triggered, this, &MainWindow::openChat);
    connect(m_saveChatAction, &QAction::triggered, this, &MainWindow::saveChat);
    connect(m_saveChatAsAction, &QAction::triggered, this, &MainWindow::saveChatAs);
    connect(m_exportMarkdownAction, &QAction::triggered, this, &MainWindow::exportMarkdown);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    connect(m_toggleThemeAction, &QAction::triggered, this, &MainWindow::toggleTheme);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupConnections()
{
    // Navigation button connections
    connect(m_newChatButton, &QPushButton::clicked, this, &MainWindow::onNewChatClicked);
    connect(m_historyButton, &QPushButton::clicked, this, &MainWindow::onHistoryClicked);
    connect(m_savedChatsButton, &QPushButton::clicked, this, &MainWindow::onSavedChatsClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    
    // Settings connections
    connect(m_settings.get(), &Settings::apiKeyChanged, this, &MainWindow::onAPIKeyChanged);
    connect(m_settings.get(), &Settings::modelChanged, this, &MainWindow::onModelChanged);
    connect(m_settings.get(), &Settings::themeChanged, this, &MainWindow::applyTheme);
    
    // API connections
    connect(m_api.get(), &OpenRouterAPI::connectionStatusChanged, this, &MainWindow::updateStatusBar);
    
    // Welcome widget connections
    connect(m_welcomeWidget.get(), &WelcomeWidget::newChatRequested, this, &MainWindow::onNewChatClicked);
    connect(m_welcomeWidget.get(), &WelcomeWidget::fileDropped, this, &MainWindow::handleFileDropped);
    connect(m_welcomeWidget.get(), &WelcomeWidget::templateSelected, this, &MainWindow::handleTemplateSelected);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                handleFileDropped(url.toLocalFile());
            }
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // Save window geometry
    if (m_settings) {
        auto& settings = m_settings->GetMutableSettings();
        settings.windowSize = size();
        if (!isMaximized()) {
            settings.windowPosition = pos();
        }
        settings.maximized = isMaximized();
    }
}

// Slot implementations
void MainWindow::newChat()
{
    m_currentFilename.clear();
    m_isModified = false;
    updateWindowTitle();
    
    // Switch to chat view if not already there
    if (m_mainSplitter->widget(1) != m_chatWidget.get()) {
        m_mainSplitter->replaceWidget(1, m_chatWidget.get());
        m_welcomeWidget->setParent(nullptr);
    }
    
    m_chatWidget->clearHistory();
    m_chatWidget->focusInput();
    
    m_statusLabel->setText("New chat started");
}

void MainWindow::openChat()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Open Chat",
        m_fileManager->getConversationsPath(),
        "Chat Files (*.json);;All Files (*)"
    );
    
    if (!filename.isEmpty()) {
        if (m_mainSplitter->widget(1) != m_chatWidget.get()) {
            m_mainSplitter->replaceWidget(1, m_chatWidget.get());
            m_welcomeWidget->setParent(nullptr);
        }
        
        m_chatWidget->loadConversation(filename);
        m_currentFilename = filename;
        m_isModified = false;
        updateWindowTitle(QFileInfo(filename).baseName());
        m_statusLabel->setText("Chat loaded");
    }
}

void MainWindow::saveChat()
{
    if (m_currentFilename.isEmpty()) {
        saveChatAs();
        return;
    }
    
    if (m_chatWidget) {
        m_chatWidget->saveConversation(m_currentFilename);
        m_isModified = false;
        updateWindowTitle();
        m_statusLabel->setText("Chat saved");
    }
}

void MainWindow::saveChatAs()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Chat As",
        m_fileManager->getConversationsPath(),
        "Chat Files (*.json);;All Files (*)"
    );
    
    if (!filename.isEmpty()) {
        if (m_chatWidget) {
            m_chatWidget->saveConversation(filename);
            m_currentFilename = filename;
            m_isModified = false;
            updateWindowTitle(QFileInfo(filename).baseName());
            m_statusLabel->setText("Chat saved");
        }
    }
}

void MainWindow::exportMarkdown()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Export as Markdown",
        m_fileManager->getExportsPath(),
        "Markdown Files (*.md);;All Files (*)"
    );
    
    if (!filename.isEmpty() && m_chatWidget) {
        m_chatWidget->exportMarkdown(filename);
        m_statusLabel->setText("Chat exported as Markdown");
    }
}

void MainWindow::openSettings()
{
    if (!m_settingsDialog) {
        m_settingsDialog = std::make_unique<SettingsDialog>(m_settings.get(), m_api.get(), this);
    }
    
    m_settingsDialog->exec();
}

void MainWindow::toggleTheme()
{
    m_darkMode = !m_darkMode;
    m_settings->SetDarkMode(m_darkMode);
    applyTheme();
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About Chatty",
        "<h3>Chatty - AI Chat Assistant</h3>"
        "<p>Version 1.0.0</p>"
        "<p>A modern, responsive AI chatbot application built with Qt Framework, "
        "featuring real-time streaming, multiple LLM providers, and rich content support.</p>"
        "<p><b>Features:</b></p>"
        "<ul>"
        "<li>Multiple LLM providers through OpenRouter API</li>"
        "<li>Real-time token streaming</li>"
        "<li>File attachments and image support</li>"
        "<li>Markdown rendering with syntax highlighting</li>"
        "<li>Modern, responsive interface</li>"
        "</ul>"
        "<p>Built with ❤️ using Qt Framework</p>"
    );
}

void MainWindow::onAPIKeyChanged(const QString &key)
{
    m_api->setAPIKey(key);
    checkAPIConnection();
}

void MainWindow::onModelChanged(const QString &model)
{
    m_api->setModel(model);
    m_modelLabel->setText(QString("Model: %1").arg(model));
}

void MainWindow::updateStatusBar()
{
    // Update connection status
    if (m_api->isRequestActive()) {
        m_statusLabel->setText("Processing...");
        m_connectionProgress->setVisible(true);
        m_connectionProgress->setRange(0, 0); // Indeterminate
    } else {
        m_statusLabel->setText("Ready");
        m_connectionProgress->setVisible(false);
    }
    
    // Update token stats
    if (m_chatWidget) {
        int totalTokens = m_chatWidget->getTotalTokens();
        double avgTPS = m_chatWidget->getAverageTokensPerSecond();
        
        if (totalTokens > 0) {
            m_tokenStatsLabel->setText(QString("Tokens: %1 | TPS: %2")
                .arg(totalTokens)
                .arg(avgTPS, 0, 'f', 1));
        } else {
            m_tokenStatsLabel->setText("Tokens: 0");
        }
    }
}

void MainWindow::checkAPIConnection()
{
    // This would typically test the API connection
    // For now, just update the status
    updateStatusBar();
}

void MainWindow::onNewChatClicked()
{
    // Update navigation state
    m_newChatButton->setChecked(true);
    m_historyButton->setChecked(false);
    m_savedChatsButton->setChecked(false);
    m_settingsButton->setChecked(false);
    
    newChat();
}

void MainWindow::onHistoryClicked()
{
    // Update navigation state
    m_newChatButton->setChecked(false);
    m_historyButton->setChecked(true);
    m_savedChatsButton->setChecked(false);
    m_settingsButton->setChecked(false);
    
    // Show history view (could be implemented as a separate widget)
    m_statusLabel->setText("History view not implemented yet");
}

void MainWindow::onSavedChatsClicked()
{
    // Update navigation state
    m_newChatButton->setChecked(false);
    m_historyButton->setChecked(false);
    m_savedChatsButton->setChecked(true);
    m_settingsButton->setChecked(false);
    
    // Show saved chats view
    m_statusLabel->setText("Saved chats view not implemented yet");
}

void MainWindow::onSettingsClicked()
{
    // Update navigation state
    m_newChatButton->setChecked(false);
    m_historyButton->setChecked(false);
    m_savedChatsButton->setChecked(false);
    m_settingsButton->setChecked(true);
    
    openSettings();
    
    // Reset navigation state
    m_newChatButton->setChecked(true);
    m_settingsButton->setChecked(false);
}

void MainWindow::applyTheme()
{
    // Load and apply the modern stylesheet
    QFile styleFile(":/styles/modern.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = styleFile.readAll();
        setStyleSheet(style);
    }
    
    // Update user profile avatar based on theme
    updateUserProfile();
}

void MainWindow::loadSettings()
{
    if (!m_settings->Load()) {
        // Create default settings
        m_settings->Reset();
        m_settings->Save();
    }
    
    const auto& settings = m_settings->GetSettings();
    
    // Apply window settings
    if (settings.rememberWindowState) {
        resize(settings.windowSize);
        if (settings.windowPosition.x() >= 0 && settings.windowPosition.y() >= 0) {
            move(settings.windowPosition);
        }
        if (settings.maximized) {
            showMaximized();
        }
    }
    
    // Apply API settings
    if (!settings.apiKey.isEmpty()) {
        m_api->setAPIKey(settings.apiKey);
        m_api->setModel(settings.selectedModel);
    }
    
    // Apply theme
    m_darkMode = settings.darkMode;
    
    // Update UI
    m_modelLabel->setText(QString("Model: %1").arg(settings.selectedModel));
    updateUserProfile();
}

void MainWindow::saveSettings()
{
    if (m_settings) {
        m_settings->Save();
    }
}

void MainWindow::updateWindowTitle(const QString &filename)
{
    QString title = "Chatty - AI Chat Assistant";
    if (!filename.isEmpty()) {
        title = QString("%1 - %2").arg(filename, title);
        if (m_isModified) {
            title = QString("*%1").arg(title);
        }
    }
    setWindowTitle(title);
}

void MainWindow::updateUserProfile()
{
    // Get user name from system or settings
    QString userName = qgetenv("USER");
    if (userName.isEmpty()) {
        userName = qgetenv("USERNAME");
    }
    if (userName.isEmpty()) {
        userName = "User";
    }
    
    m_userNameLabel->setText(userName);
    
    // Set avatar with first letter of name
    QString avatarText = userName.left(1).toUpper();
    m_userAvatarLabel->setText(avatarText);
    
    // Update status based on API connection
    if (m_settings->GetAPIKey().isEmpty()) {
        m_userStatusLabel->setText("Configure API key");
    } else if (m_api->isRequestActive()) {
        m_userStatusLabel->setText("Thinking...");
    } else {
        m_userStatusLabel->setText("Ready to chat");
    }
}

void MainWindow::handleFileDropped(const QString &filePath)
{
    // Switch to chat view and handle file
    if (m_mainSplitter->widget(1) != m_chatWidget.get()) {
        m_mainSplitter->replaceWidget(1, m_chatWidget.get());
        m_welcomeWidget->setParent(nullptr);
    }
    
    // The ChatWidget will handle the file attachment
    m_chatWidget->HandleFileDrops({filePath});
    m_statusLabel->setText("File attached");
}

void MainWindow::handleTemplateSelected(const QString &template_)
{
    // Switch to chat view and set template
    if (m_mainSplitter->widget(1) != m_chatWidget.get()) {
        m_mainSplitter->replaceWidget(1, m_chatWidget.get());
        m_welcomeWidget->setParent(nullptr);
    }
    
    // This would set the template text in the input
    // The ChatWidget will need a method to set initial text
    m_statusLabel->setText("Template applied");
}

// Placeholder implementations for edit menu
void MainWindow::copy()
{
    // Will be implemented when ChatWidget is ready
}

void MainWindow::paste()
{
    // Will be implemented when ChatWidget is ready
}

void MainWindow::selectAll()
{
    // Will be implemented when ChatWidget is ready
} 