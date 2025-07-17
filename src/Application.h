#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <string>

class ChatInterface;
class OpenRouterAPI;
class Settings;
class ThemeManager;
class FileManager;

class Application {
public:
    Application();
    ~Application();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
private:
    // Core components
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<ChatInterface> m_chatInterface;
    std::unique_ptr<OpenRouterAPI> m_api;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<ThemeManager> m_themeManager;
    std::unique_ptr<FileManager> m_fileManager;
    
    // Window properties
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
    std::string m_windowTitle = "Chatty - AI Chat Assistant";
    
    // Application state
    bool m_running = true;
    bool m_showDemo = false;
    bool m_showSettings = false;
    bool m_showAbout = false;
    
    // Performance tracking
    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;
    int m_frameCount = 0;
    float m_fpsTimer = 0.0f;
    float m_fps = 0.0f;
    
    // Methods
    bool InitializeWindow();
    bool InitializeImGui();
    void InitializeComponents();
    
    void Update();
    void Render();
    void RenderMainMenu();
    void RenderStatusBar();
    void RenderSettingsWindow();
    void RenderAboutWindow();
    
    void HandleWindowResize();
    void UpdatePerformanceMetrics();
    
    // Callbacks
    static void WindowSizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void DropCallback(GLFWwindow* window, int count, const char** paths);
    
    // Utility
    void SetupStyle();
    void LoadSettings();
    void SaveSettings();
}; 