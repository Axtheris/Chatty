#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QAction>
#include <QTimer>
#include <memory>

class ChatWidget;
class OpenRouterAPI;
class Settings;
class SettingsDialog;
class FileManager;

QT_BEGIN_NAMESPACE
class QTextEdit;
class QLineEdit;
class QPushButton;
class QScrollArea;
class QFrame;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void newChat();
    void openChat();
    void saveChat();
    void saveChatAs();
    void exportMarkdown();
    void openSettings();
    void toggleTheme();
    void showAbout();
    void onAPIKeyChanged(const QString &key);
    void onModelChanged(const QString &model);
    void updateStatusBar();
    void checkAPIConnection();
    
    // Sidebar slots
    void onNewChatClicked();
    void onHistoryClicked();
    void onSavedChatsClicked();
    void onSettingsClicked();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupSidebar();
    void setupActions();
    void setupConnections();
    void applyTheme();
    void loadSettings();
    void saveSettings();
    void updateWindowTitle(const QString &filename = QString());
    void updateUserProfile();
    
    // Core components
    std::unique_ptr<ChatWidget> m_chatWidget;
    std::unique_ptr<OpenRouterAPI> m_api;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<SettingsDialog> m_settingsDialog;
    std::unique_ptr<FileManager> m_fileManager;
    
    // UI components
    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;
    QSplitter *m_mainSplitter;
    
    // Sidebar components
    QFrame *m_sidebarFrame;
    QVBoxLayout *m_sidebarLayout;
    QFrame *m_userProfileFrame;
    QVBoxLayout *m_userProfileLayout;
    QLabel *m_userAvatarLabel;
    QLabel *m_userNameLabel;
    QLabel *m_userStatusLabel;
    
    // Navigation
    QWidget *m_navigationWidget;
    QVBoxLayout *m_navigationLayout;
    QPushButton *m_newChatButton;
    QPushButton *m_historyButton;
    QPushButton *m_savedChatsButton;
    QPushButton *m_settingsButton;
    
    // Menu actions
    QAction *m_newChatAction;
    QAction *m_openChatAction;
    QAction *m_saveChatAction;
    QAction *m_saveChatAsAction;
    QAction *m_exportMarkdownAction;
    QAction *m_settingsAction;
    QAction *m_toggleThemeAction;
    QAction *m_aboutAction;
    QAction *m_exitAction;
    
    // Status bar
    QLabel *m_statusLabel;
    QLabel *m_modelLabel;
    QLabel *m_tokenStatsLabel;
    QProgressBar *m_connectionProgress;
    QTimer *m_statusTimer;
    
    // State
    QString m_currentFilename;
    bool m_isModified = false;
    bool m_darkMode = true;
    
    // Performance tracking
    int m_totalTokens = 0;
    double m_averageTPS = 0.0;
    int m_messageCount = 0;
}; 