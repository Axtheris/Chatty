#pragma once

#include "Message.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

class OpenRouterAPI;
class MarkdownRenderer;
class FileManager;

class ChatInterface {
public:
    ChatInterface(OpenRouterAPI* api, MarkdownRenderer* renderer, FileManager* fileManager);
    ~ChatInterface();
    
    void Render();
    void Update(float deltaTime);
    
    // Message management
    void AddMessage(const Message& message);
    void ClearHistory();
    void LoadHistory();
    void SaveHistory();
    
    // Input handling
    void HandleKeyboardInput();
    void HandleFileDrops(const std::vector<std::string>& filePaths);
    
    // UI state
    void SetFocusOnInput() { m_focusInput = true; }
    bool IsInputFocused() const { return m_inputFocused; }
    
private:
    OpenRouterAPI* m_api;
    MarkdownRenderer* m_markdownRenderer;
    FileManager* m_fileManager;
    
    // Chat data
    std::vector<Message> m_messages;
    Message m_currentMessage;
    
    // UI state
    char m_inputBuffer[4096] = {0};
    bool m_focusInput = false;
    bool m_inputFocused = false;
    bool m_autoScroll = true;
    bool m_showTokenStats = true;
    float m_scrollToBottom = 0.0f;
    
    // Streaming state
    bool m_isStreaming = false;
    std::string m_streamingContent;
    Message* m_streamingMessage = nullptr;
    
    // Attachments
    std::vector<std::shared_ptr<Attachment>> m_pendingAttachments;
    
    // Animation
    float m_messageAnimTimer = 0.0f;
    float m_typingIndicatorTimer = 0.0f;
    
    // Layout
    float m_chatAreaHeight = 0.0f;
    float m_inputAreaHeight = 100.0f;
    float m_sidebarWidth = 200.0f;
    bool m_showSidebar = false;
    
    // Methods
    void RenderChatArea();
    void RenderMessage(const Message& message, int index);
    void RenderUserMessage(const Message& message);
    void RenderAssistantMessage(const Message& message);
    void RenderSystemMessage(const Message& message);
    void RenderInputArea();
    void RenderSidebar();
    void RenderAttachmentPreview(const Attachment& attachment);
    void RenderAttachmentPreviews();
    void RenderTypingIndicator();
    void RenderTokenStats();
    
    // Message rendering helpers
    void RenderMessageHeader(const Message& message);
    void RenderMessageContent(const Message& message);
    void RenderMessageAttachments(const Message& message);
    void RenderMessageFooter(const Message& message);
    
    // Input handling
    void ProcessInput();
    void SendMessage();
    void AddAttachment(const std::string& filePath);
    void RemoveAttachment(int index);
    void ClearAttachments();
    
    // Streaming callbacks
    void OnStreamToken(const std::string& token);
    void OnStreamComplete(bool success);
    void OnStreamError(const std::string& error);
    
    // Utility
    void ScrollToBottom();
    void UpdateAnimations(float deltaTime);
    std::string GetRelativeTimeString(std::chrono::system_clock::time_point time);
    ImVec4 GetMessageColor(MessageRole role);
    ImVec4 GetStatusColor(MessageStatus status);
    
    // File operations
    void SaveConversation(const std::string& filename);
    void LoadConversation(const std::string& filename);
    void ExportMarkdown(const std::string& filename);
}; 