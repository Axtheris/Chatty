#pragma once

#include "Message.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QStringList>
#include <vector>
#include <memory>
#include <atomic>

struct ModelInfo {
    QString id;
    QString name;
    QString description;
    QString provider;
    double costPerToken;
    int maxTokens;
    bool supportsImages;
    bool supportsFiles;
    
    ModelInfo(const QString& modelId, const QString& modelName)
        : id(modelId), name(modelName), costPerToken(0.0), maxTokens(4096), 
          supportsImages(false), supportsFiles(false) {}
};

class OpenRouterAPI : public QObject {
    Q_OBJECT

public:
    
    explicit OpenRouterAPI(QObject *parent = nullptr);
    ~OpenRouterAPI();
    
    // Configuration
    void setAPIKey(const QString& apiKey);
    void setModel(const QString& modelId);
    void setBaseURL(const QString& url = "https://openrouter.ai/api/v1");
    
    // Model management
    void refreshModels();
    const std::vector<ModelInfo>& getModels() const { return m_models; }
    const ModelInfo* getCurrentModel() const;
    
    // Chat functionality
    void sendMessage(const std::vector<Message>& conversation);
    void stopCurrentRequest();
    bool isRequestActive() const { return m_requestActive; }
    
    // Statistics
    double getTokensPerSecond() const { return m_tokensPerSecond; }
    int getTotalTokensUsed() const { return m_totalTokensUsed; }
    double getEstimatedCost() const { return m_estimatedCost; }

signals:
    void modelsRefreshed(bool success);
    void streamReceived(const QString& content);
    void streamCompleted(bool success);
    void streamError(const QString& error);
    void connectionStatusChanged(bool connected);

private slots:
    void onModelsReplyFinished();
    void onChatReplyFinished();
    void onChatReplyReadyRead();
    void onNetworkError(QNetworkReply::NetworkError error);
    
private:
    QString m_apiKey;
    QString m_modelId = "openai/gpt-3.5-turbo";
    QString m_baseURL = "https://openrouter.ai/api/v1";
    
    std::vector<ModelInfo> m_models;
    std::atomic<bool> m_requestActive{false};
    std::atomic<bool> m_shouldStop{false};
    
    // Statistics
    std::atomic<double> m_tokensPerSecond{0.0};
    std::atomic<int> m_totalTokensUsed{0};
    std::atomic<double> m_estimatedCost{0.0};
    
    // Qt Network components
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
    QTimer *m_streamTimer;
    
    // Streaming state
    QString m_streamBuffer;
    std::chrono::steady_clock::time_point m_streamStartTime;
    int m_tokenCount = 0;
    
    // Internal methods
    bool parseModelsResponse(const QByteArray& response);
    QJsonObject prepareRequestPayload(const std::vector<Message>& conversation);
    void processStreamChunk(const QString& chunk);
    QNetworkRequest createRequest(const QString& endpoint);
    void updateTokenStats();
}; 