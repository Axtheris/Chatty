#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QTimer>
#include <QPropertyAnimation>

class Settings;
class OpenRouterAPI;

QT_BEGIN_NAMESPACE
class QGraphicsDropShadowEffect;
QT_END_NAMESPACE

class WelcomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWidget(Settings* settings, OpenRouterAPI* api, QWidget *parent = nullptr);
    ~WelcomeWidget();
    
    void updateGreeting();
    void updateStats();
    void refreshRecentFiles();

signals:
    void newChatRequested();
    void fileDropped(const QString& filePath);
    void templateSelected(const QString& template_);
    void recentFileOpened(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onNewChatClicked();
    void onTemplateClicked();
    void onRecentFileClicked();
    void animateCards();

private:
    void setupUI();
    void setupHeader();
    void setupQuickActions();
    void setupRecentFiles();
    void setupTemplates();
    void setupStats();
    void applyTheme();
    void startAnimations();
    
    // Card creation helpers
    QFrame* createCard(const QString& title, const QString& subtitle, const QString& iconPath);
    QFrame* createStatsCard(const QString& title, const QString& value, const QString& trend);
    QFrame* createTemplateCard(const QString& title, const QString& description, const QString& template_);
    QFrame* createRecentFileCard(const QString& filename, const QString& path, const QDateTime& modified);
    
    Settings* m_settings;
    OpenRouterAPI* m_api;
    
    // Main layout
    QScrollArea* m_scrollArea;
    QWidget* m_scrollWidget;
    QVBoxLayout* m_mainLayout;
    
    // Header section
    QFrame* m_headerFrame;
    QVBoxLayout* m_headerLayout;
    QLabel* m_greetingLabel;
    QLabel* m_subtitleLabel;
    QLabel* m_timeLabel;
    
    // Quick actions section
    QFrame* m_quickActionsFrame;
    QVBoxLayout* m_quickActionsLayout;
    QLabel* m_quickActionsTitleLabel;
    QHBoxLayout* m_actionsLayout;
    QPushButton* m_newChatButton;
    QPushButton* m_uploadFileButton;
    QPushButton* m_settingsButton;
    
    // Recent files section
    QFrame* m_recentFilesFrame;
    QVBoxLayout* m_recentFilesLayout;
    QLabel* m_recentFilesTitleLabel;
    QGridLayout* m_recentFilesGrid;
    
    // Templates section
    QFrame* m_templatesFrame;
    QVBoxLayout* m_templatesLayout;
    QLabel* m_templatesTitleLabel;
    QGridLayout* m_templatesGrid;
    
    // Stats section
    QFrame* m_statsFrame;
    QVBoxLayout* m_statsLayout;
    QLabel* m_statsTitleLabel;
    QHBoxLayout* m_statsCardsLayout;
    
    // Animation
    QTimer* m_animationTimer;
    QPropertyAnimation* m_fadeAnimation;
    std::vector<QPropertyAnimation*> m_cardAnimations;
    
    // State
    bool m_isVisible = false;
    int m_animationDelay = 0;
    
    // Templates
    struct ChatTemplate {
        QString title;
        QString description;
        QString prompt;
        QString icon;
    };
    
    std::vector<ChatTemplate> m_templates = {
        {"Code Review", "Analyze and review code for improvements", "Please review this code and suggest improvements:\n\n", ":/icons/code.png"},
        {"Explain Code", "Get detailed explanations of complex code", "Please explain how this code works:\n\n", ":/icons/explain.png"},
        {"Debug Help", "Help troubleshoot and fix bugs", "I'm having trouble with this code. Can you help me debug it?\n\n", ":/icons/debug.png"},
        {"Documentation", "Generate documentation for code", "Please generate documentation for this code:\n\n", ":/icons/docs.png"},
        {"Scala Expert", "Specialized help with Scala programming", "I need help with Scala. Here's my question:\n\n", ":/icons/scala.png"},
        {"General Chat", "Start a general conversation", "Hello! I'd like to chat about: ", ":/icons/chat.png"}
    };
}; 