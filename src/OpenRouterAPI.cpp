#include "OpenRouterAPI.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>
#include <QSslConfiguration>
#include <QUrl>
#include <QUrlQuery>

OpenRouterAPI::OpenRouterAPI(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_streamTimer = new QTimer(this);
    m_streamTimer->setSingleShot(false);
    
    // Configure SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    
    // Initialize default models
    initializeDefaultModels();
}

OpenRouterAPI::~OpenRouterAPI()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void OpenRouterAPI::setAPIKey(const QString& apiKey)
{
    m_apiKey = apiKey;
    emit connectionStatusChanged(!apiKey.isEmpty());
}

void OpenRouterAPI::setModel(const QString& modelId)
{
    m_modelId = modelId;
}

void OpenRouterAPI::setBaseURL(const QString& url)
{
    m_baseURL = url;
}

void OpenRouterAPI::refreshModels()
{
    if (m_apiKey.isEmpty()) {
        qWarning() << "Cannot refresh models: API key not set";
        emit modelsRefreshed(false);
        return;
    }
    
    QUrl url(m_baseURL + "/models");
    QNetworkRequest request = createRequest(url.toString());
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &OpenRouterAPI::onModelsReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &OpenRouterAPI::onNetworkError);
}

const ModelInfo* OpenRouterAPI::getCurrentModel() const
{
    for (const auto& model : m_models) {
        if (model.id == m_modelId) {
            return &model;
        }
    }
    return nullptr;
}

void OpenRouterAPI::sendMessage(const std::vector<Message>& conversation)
{
    if (m_apiKey.isEmpty()) {
        emit streamError("API key not configured");
        return;
    }
    
    if (m_requestActive) {
        qWarning() << "Request already in progress";
        return;
    }
    
    m_requestActive = true;
    m_shouldStop = false;
    m_streamStartTime = std::chrono::steady_clock::now();
    m_tokenCount = 0;
    
    QUrl url(m_baseURL + "/chat/completions");
    QNetworkRequest request = createRequest(url.toString());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject payload = prepareRequestPayload(conversation);
    QJsonDocument doc(payload);
    
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    connect(m_currentReply, &QNetworkReply::readyRead, this, &OpenRouterAPI::onChatReplyReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &OpenRouterAPI::onChatReplyFinished);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &OpenRouterAPI::onNetworkError);
}

void OpenRouterAPI::stopCurrentRequest()
{
    m_shouldStop = true;
    if (m_currentReply) {
        m_currentReply->abort();
    }
}

QNetworkRequest OpenRouterAPI::createRequest(const QString& endpoint)
{
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    request.setRawHeader("User-Agent", "Chatty/1.0.0");
    request.setRawHeader("Accept", "application/json");
    
    return request;
}

QJsonObject OpenRouterAPI::prepareRequestPayload(const std::vector<Message>& conversation)
{
    QJsonObject payload;
    payload["model"] = m_modelId;
    payload["stream"] = true;
    payload["temperature"] = 0.7;
    payload["max_tokens"] = 2048;
    
    QJsonArray messages;
    for (const auto& msg : conversation) {
        QJsonObject messageObj;
        
        // Convert role
        switch (msg.role) {
            case MessageRole::User:
                messageObj["role"] = "user";
                break;
            case MessageRole::Assistant:
                messageObj["role"] = "assistant";
                break;
            case MessageRole::System:
                messageObj["role"] = "system";
                break;
        }
        
        // Add content
        if (msg.attachments.empty()) {
            messageObj["content"] = msg.content;
        } else {
            // Handle multimodal content (text + images)
            QJsonArray contentArray;
            
            // Add text content
            if (!msg.content.isEmpty()) {
                QJsonObject textContent;
                textContent["type"] = "text";
                textContent["text"] = msg.content;
                contentArray.append(textContent);
            }
            
            // Add image attachments
            for (const auto& attachment : msg.attachments) {
                if (attachment->isImage && !attachment->data.isEmpty()) {
                    QJsonObject imageContent;
                    imageContent["type"] = "image_url";
                    
                    QJsonObject imageUrl;
                    QString base64Data = QString("data:%1;base64,%2")
                        .arg(attachment->mimeType)
                        .arg(QString::fromLatin1(attachment->data.toBase64()));
                    imageUrl["url"] = base64Data;
                    
                    imageContent["image_url"] = imageUrl;
                    contentArray.append(imageContent);
                }
            }
            
            messageObj["content"] = contentArray;
        }
        
        messages.append(messageObj);
    }
    
    payload["messages"] = messages;
    
    return payload;
}

void OpenRouterAPI::onModelsReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    bool success = false;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        success = parseModelsResponse(data);
    } else {
        qWarning() << "Models request failed:" << reply->errorString();
    }
    
    emit modelsRefreshed(success);
    reply->deleteLater();
}

void OpenRouterAPI::onChatReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    m_requestActive = false;
    
    bool success = (reply->error() == QNetworkReply::NoError && !m_shouldStop);
    
    if (!success && reply->error() != QNetworkReply::OperationCanceledError) {
        QString errorMsg = QString("Request failed: %1").arg(reply->errorString());
        emit streamError(errorMsg);
    }
    
    emit streamCompleted(success);
    updateTokenStats();
    
    m_currentReply = nullptr;
    reply->deleteLater();
}

void OpenRouterAPI::onChatReplyReadyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || m_shouldStop) return;
    
    QByteArray newData = reply->readAll();
    m_streamBuffer.append(QString::fromUtf8(newData));
    
    // Process complete chunks
    QStringList lines = m_streamBuffer.split('\n');
    m_streamBuffer = lines.takeLast(); // Keep the incomplete line in buffer
    
    for (const QString& line : lines) {
        if (!line.isEmpty()) {
            processStreamChunk(line.trimmed());
        }
    }
}

void OpenRouterAPI::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString errorMsg = QString("Network error: %1").arg(reply->errorString());
    qWarning() << errorMsg;
    
    if (reply == m_currentReply) {
        m_requestActive = false;
        emit streamError(errorMsg);
    }
}

void OpenRouterAPI::processStreamChunk(const QString& chunk)
{
    // Handle Server-Sent Events format
    if (chunk.startsWith("data: ")) {
        QString data = chunk.mid(6); // Remove "data: " prefix
        
        if (data == "[DONE]") {
            // Stream finished
            return;
        }
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &error);
        
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Failed to parse stream chunk:" << error.errorString();
            return;
        }
        
        QJsonObject obj = doc.object();
        
        if (obj.contains("choices")) {
            QJsonArray choices = obj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject choice = choices[0].toObject();
                QJsonObject delta = choice["delta"].toObject();
                
                if (delta.contains("content")) {
                    QString content = delta["content"].toString();
                    if (!content.isEmpty()) {
                        emit streamReceived(content);
                        m_tokenCount++;
                        updateTokenStats();
                    }
                }
            }
        }
        
        // Update usage statistics if available
        if (obj.contains("usage")) {
            QJsonObject usage = obj["usage"].toObject();
            if (usage.contains("total_tokens")) {
                m_totalTokensUsed = usage["total_tokens"].toInt();
            }
        }
    }
}

void OpenRouterAPI::updateTokenStats()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_streamStartTime);
    
    if (duration.count() > 0 && m_tokenCount > 0) {
        m_tokensPerSecond = (m_tokenCount * 1000.0) / duration.count();
    }
}

bool OpenRouterAPI::parseModelsResponse(const QByteArray& response)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse models response:" << error.errorString();
        return false;
    }
    
    QJsonObject obj = doc.object();
    if (!obj.contains("data")) {
        qWarning() << "Models response missing 'data' field";
        return false;
    }
    
    QJsonArray data = obj["data"].toArray();
    m_models.clear();
    
    for (const QJsonValue& value : data) {
        QJsonObject modelObj = value.toObject();
        
        ModelInfo model(
            modelObj["id"].toString(),
            modelObj["name"].toString()
        );
        
        model.description = modelObj["description"].toString();
        model.provider = modelObj["owned_by"].toString();
        
        // Parse context length
        if (modelObj.contains("context_length")) {
            model.maxTokens = modelObj["context_length"].toInt();
        }
        
        // Parse pricing if available
        if (modelObj.contains("pricing")) {
            QJsonObject pricing = modelObj["pricing"].toObject();
            if (pricing.contains("prompt")) {
                model.costPerToken = pricing["prompt"].toString().toDouble();
            }
        }
        
        // Check capabilities
        model.supportsImages = modelObj["modalities"].toArray().contains("vision");
        model.supportsFiles = true; // Most models support text files
        
        m_models.push_back(model);
    }
    
    qDebug() << "Loaded" << m_models.size() << "models";
    return true;
}

void OpenRouterAPI::initializeDefaultModels()
{
    // Add some popular models as defaults
    m_models = {
        ModelInfo("openai/gpt-4", "GPT-4"),
        ModelInfo("openai/gpt-3.5-turbo", "GPT-3.5 Turbo"),
        ModelInfo("anthropic/claude-2", "Claude 2"),
        ModelInfo("anthropic/claude-instant-v1", "Claude Instant"),
        ModelInfo("meta-llama/llama-2-70b-chat", "Llama 2 70B"),
        ModelInfo("google/palm-2-chat-bison", "PaLM 2 Chat"),
        ModelInfo("cohere/command", "Cohere Command")
    };
    
    // Set default properties
    for (auto& model : m_models) {
        model.maxTokens = 4096;
        model.supportsFiles = true;
        
        if (model.id.contains("gpt-4")) {
            model.maxTokens = 8192;
            model.supportsImages = true;
            model.costPerToken = 0.00003;
        } else if (model.id.contains("gpt-3.5")) {
            model.costPerToken = 0.000002;
        } else if (model.id.contains("claude")) {
            model.maxTokens = 100000;
            model.costPerToken = 0.000008;
        }
    }
} 