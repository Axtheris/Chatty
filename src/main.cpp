#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include "MainWindow.h"
#include "Settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Chatty");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Chatty Team");
    app.setOrganizationDomain("chatty.ai");
    
    // Enable high DPI support
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Set up application directories
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    
    // Load settings
    Settings settings;
    if (!settings.Load()) {
        qWarning() << "Failed to load settings, using defaults";
    }
    
    // Apply theme based on settings
    if (settings.IsDarkMode()) {
        app.setStyle(QStyleFactory::create("Fusion"));
        
        // Dark theme palette
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        app.setPalette(darkPalette);
    }
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    // Handle application shutdown
    QObject::connect(&app, &QApplication::aboutToQuit, [&settings]() {
        settings.Save();
    });
    
    return app.exec();
} 