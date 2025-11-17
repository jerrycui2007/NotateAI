/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "geminiservice.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <chrono>
#include <thread>

#include "global/concurrency/concurrent.h"
#include "log.h"
#include "context/iglobalcontext.h"
#include "notation/inotation.h"
#include "notation/inotationelements.h"
#include "engraving/dom/score.h"
#include "engraving/dom/masterscore.h"
#include "importexport/musicxml/internal/musicxml/export/exportmusicxml.h"
#include "io/buffer.h"

using namespace mu::notateai;
using namespace muse;
using namespace muse::network;

static const QString GEMINI_API_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent";

// Hardcoded system prompt - defines the AI's role and behavior
static const QString SYSTEM_PROMPT = R"(You are a helpful AI assistant integrated into NotateAI, a music notation software.
You help users with questions about music theory, notation, and using the software. Your role is basically Cursor but for music notation.
If the user sends an off-topic message, politely inform them that you can only assist with music notation related queries.

Your expertise includes:
- Music theory (harmony, counterpoint, form, analysis)
- Music notation and engraving
- Composition and arranging
- Using the NotateAI software

When responding:
- Be clear, concise, and educational
- Use musical examples when helpful
- Be encouraging and supportive
- If you don't know something, admit it rather than making up information.

This is the end of the system prompt. The rest of the prompt is the message sent by the user.

)"

;

void GeminiService::sendMessage(const QString& userMessage)
{
    LOGI() << "GeminiService::sendMessage called with message: " << userMessage;

    // STEP 3 VERIFICATION: Test score data extraction
    QString scoreData = extractScoreDataAsMusicXML();
    LOGI() << "=== SCORE DATA EXTRACTION TEST ===";
    LOGI() << "Score data length:" << scoreData.length();
    LOGI() << "First 500 chars:" << scoreData.left(500);
    LOGI() << "===================================";

    auto callback = [this](GeminiResponse response) {
        LOGI() << "Callback lambda invoked! Success: " << response.success;
        LOGI() << "Callback: About to emit responseReceived signal";

        // Emit the signal - Qt will handle cross-thread communication automatically
        emit responseReceived(response);

        LOGI() << "Callback: responseReceived signal emitted";
    };

    LOGI() << "Starting concurrent task...";
    Concurrent::run(this, &GeminiService::th_sendMessageDirect, userMessage, callback);
    LOGI() << "Concurrent task started";
}

void GeminiService::th_sendMessage(const QString& userMessage, std::function<void(GeminiResponse)> callback) const
{
    TRACEFUNC;

    GeminiResponse response;

    // Check if API key is configured
    QString apiKey = configuration()->geminiApiKey();
    if (apiKey.isEmpty()) {
        LOGE() << "Gemini API key is not configured";
        response.success = false;
        response.errorMessage = "API key not configured. Please add your Gemini API key in Preferences.";
        callback(response);
        return;
    }

    // Build the request URL with API key
    QUrl requestUrl(GEMINI_API_ENDPOINT);
    QUrlQuery query;
    query.addQueryItem("key", apiKey);
    requestUrl.setQuery(query);

    // Create network manager
    INetworkManagerPtr networkManager = networkManagerCreator()->makeNetworkManager();

    // Build request JSON
    QByteArray requestJson = buildRequestJson(userMessage);

    // Setup request headers
    RequestHeaders headers;
    headers.knownHeaders[QNetworkRequest::ContentTypeHeader] = "application/json";

    // Prepare buffers for request and response
    QBuffer requestBuffer;
    requestBuffer.setData(requestJson);
    requestBuffer.open(QIODevice::ReadOnly);

    QBuffer responseBuffer;

    OutgoingDevice outgoingDevice(&requestBuffer);

    LOGI() << "Sending request to Gemini API...";
    LOGD() << "Request URL: " << requestUrl.toString();
    LOGD() << "Request JSON: " << requestJson;

    // Send POST request
    Ret result = networkManager->post(requestUrl, &outgoingDevice, &responseBuffer, headers);

    if (!result) {
        LOGE() << "Network request failed: " << result.toString();
        LOGE() << "Error code: " << result.code();
        LOGE() << "Error text: " << result.text();

        // Check if there's any response data even on error
        QByteArray errorData = responseBuffer.data();
        if (!errorData.isEmpty()) {
            LOGE() << "Response data on error: " << errorData;
        }

        response.success = false;

        // Provide user-friendly error messages
        if (result.code() == static_cast<int>(Ret::Code::UnknownError)) {
            response.errorMessage = "Network error. Please check your internet connection.";
        } else {
            response.errorMessage = QString("Request failed: %1").arg(QString::fromStdString(result.text()));
        }

        callback(response);
        return;
    }

    // Parse response
    QByteArray responseData = responseBuffer.data();
    LOGD() << "Response received: " << responseData;

    QJsonParseError parseError;
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        LOGE() << "Failed to parse JSON response: " << parseError.errorString();
        response.success = false;
        response.errorMessage = "Failed to parse API response.";
        callback(response);
        return;
    }

    // Parse the response
    response = parseResponse(responseDoc);

    if (response.success) {
        LOGI() << "Successfully received AI response";
    } else {
        LOGW() << "API returned error: " << response.errorMessage;
    }

    callback(response);
}

QString GeminiService::extractScoreDataAsMusicXML() const
{
    // Get current notation from global context (call Inject as a function)
    notation::INotationPtr notation = m_globalContext()->currentNotation();

    if (!notation) {
        return QString("No score currently open");
    }

    // Get the underlying Score object
    mu::engraving::Score* score = notation->elements()->msScore();

    if (!score) {
        return QString("Score data unavailable");
    }

    // Export to MusicXML format using muse::io::Buffer
    muse::io::Buffer buffer;
    buffer.open(muse::io::IODevice::OpenMode::WriteOnly);

    // Use the MusicXML writer to export
    bool success = mu::iex::musicxml::saveXml(score, &buffer);

    if (!success) {
        return QString("Error exporting score to MusicXML");
    }

    // Convert buffer to string
    muse::ByteArray data = buffer.data();
    QString musicXml = QString::fromUtf8(data.constChar(), data.size());

    return musicXml;
}

QByteArray GeminiService::buildRequestJson(const QString& userMessage) const
{
    QJsonObject requestObj;

    // Add hardcoded system instruction
    QJsonObject systemInstructionObj;
    QJsonArray systemPartsArray;
    QJsonObject systemPartObj;
    systemPartObj["text"] = SYSTEM_PROMPT;
    systemPartsArray.append(systemPartObj);
    systemInstructionObj["parts"] = systemPartsArray;
    requestObj["systemInstruction"] = systemInstructionObj;

    // Build the contents array
    QJsonArray contentsArray;
    QJsonObject contentObj;

    QJsonArray partsArray;
    QJsonObject partObj;
    partObj["text"] = userMessage;
    partsArray.append(partObj);

    contentObj["parts"] = partsArray;
    contentsArray.append(contentObj);

    requestObj["contents"] = contentsArray;

    QJsonDocument doc(requestObj);
    return doc.toJson(QJsonDocument::Compact);
}

GeminiService::GeminiResponse GeminiService::parseResponse(const QJsonDocument& responseDoc) const
{
    GeminiResponse response;

    QJsonObject rootObj = responseDoc.object();

    // Check for error in response
    if (rootObj.contains("error")) {
        QJsonObject errorObj = rootObj["error"].toObject();
        QString errorMessage = errorObj["message"].toString();
        int errorCode = errorObj["code"].toInt();

        LOGW() << "API error code: " << errorCode << ", message: " << errorMessage;

        response.success = false;

        // Provide user-friendly error messages
        if (errorCode == 401 || errorCode == 403) {
            response.errorMessage = "API authentication failed. Please check your API key in Preferences.";
        } else if (errorCode == 429) {
            response.errorMessage = "API rate limit exceeded. Please wait and try again.";
        } else {
            response.errorMessage = QString("API error: %1").arg(errorMessage);
        }

        return response;
    }

    // Parse successful response
    if (!rootObj.contains("candidates")) {
        LOGW() << "Response missing 'candidates' field";
        response.success = false;
        response.errorMessage = "Invalid API response format.";
        return response;
    }

    QJsonArray candidates = rootObj["candidates"].toArray();
    if (candidates.isEmpty()) {
        LOGW() << "No candidates in response";
        response.success = false;
        response.errorMessage = "No response generated.";
        return response;
    }

    QJsonObject firstCandidate = candidates[0].toObject();
    QJsonObject content = firstCandidate["content"].toObject();
    QJsonArray parts = content["parts"].toArray();

    if (parts.isEmpty()) {
        LOGW() << "No parts in candidate response";
        response.success = false;
        response.errorMessage = "Empty response from API.";
        return response;
    }

    QJsonObject firstPart = parts[0].toObject();
    QString text = firstPart["text"].toString();

    if (text.isEmpty()) {
        LOGW() << "Empty text in response";
        response.success = false;
        response.errorMessage = "Empty response from API.";
        return response;
    }

    response.success = true;
    response.responseText = text;

    return response;
}

void GeminiService::th_sendMessageDirect(const QString& userMessage, std::function<void(GeminiResponse)> callback) const
{
    TRACEFUNC;

    GeminiResponse response;

    // Check if API key is configured
    QString apiKey = configuration()->geminiApiKey();
    if (apiKey.isEmpty()) {
        LOGE() << "Gemini API key is not configured";
        response.success = false;
        response.errorMessage = "API key not configured. Please add your Gemini API key in Preferences.";
        callback(response);
        return;
    }

    // Build the request URL with API key
    QUrl requestUrl(GEMINI_API_ENDPOINT);
    QUrlQuery query;
    query.addQueryItem("key", apiKey);
    requestUrl.setQuery(query);

    // Build request JSON
    QByteArray requestJson = buildRequestJson(userMessage);

    LOGI() << "Sending request to Gemini API using direct QNetworkAccessManager...";
    LOGD() << "Request URL: " << requestUrl.toString();
    LOGD() << "Request JSON: " << requestJson;

    // Create network request
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    // Create network manager
    QNetworkAccessManager* manager = new QNetworkAccessManager();

    // Handle SSL errors - ignore them for now (for debugging)
    QObject::connect(manager, &QNetworkAccessManager::sslErrors,
                     [](QNetworkReply* reply, const QList<QSslError>& errors) {
        LOGW() << "SSL errors occurred:";
        for (const QSslError& error : errors) {
            LOGW() << "  - " << error.errorString();
        }
        // Ignore SSL errors for Google's API
        reply->ignoreSslErrors();
    });

    // Send POST request
    QNetworkReply* reply = manager->post(request, requestJson);

    // Also connect SSL errors directly to the reply
    QObject::connect(reply, QOverload<const QList<QSslError>&>::of(&QNetworkReply::sslErrors),
                     [](const QList<QSslError>& errors) {
        LOGW() << "SSL errors on reply:";
        for (const QSslError& error : errors) {
            LOGW() << "  - " << error.errorString();
        }
    });

    // Wait for reply
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool isTimeout = false;
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&isTimeout, reply]() {
        isTimeout = true;
        reply->abort();
    });

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timeoutTimer.start(60000); // 60 second timeout
    loop.exec();

    if (isTimeout) {
        LOGE() << "Request timed out";
        response.success = false;
        response.errorMessage = "Request timed out. Please try again.";
        reply->deleteLater();
        manager->deleteLater();
        callback(response);
        return;
    }

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        LOGE() << "Network error: " << reply->errorString();
        LOGE() << "Error code: " << reply->error();

        // Log HTTP status code
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid()) {
            LOGE() << "HTTP status code: " << statusCode.toInt();
        }

        // Log response data even on error
        QByteArray errorResponseData = reply->readAll();
        if (!errorResponseData.isEmpty()) {
            LOGE() << "Error response data: " << errorResponseData;
        }

        response.success = false;

        // Provide user-friendly error messages
        switch (reply->error()) {
        case QNetworkReply::AuthenticationRequiredError:
            response.errorMessage = "API authentication failed. Please check your API key in Preferences.";
            break;
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::NetworkSessionFailedError:
            response.errorMessage = "Network error. Please check your internet connection.";
            break;
        default:
            response.errorMessage = QString("Request failed: %1").arg(reply->errorString());
            break;
        }

        reply->deleteLater();
        manager->deleteLater();
        callback(response);
        return;
    }

    // Read response
    QByteArray responseData = reply->readAll();
    LOGD() << "Response received: " << responseData;

    reply->deleteLater();
    manager->deleteLater();

    // Parse response
    QJsonParseError parseError;
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        LOGE() << "Failed to parse JSON response: " << parseError.errorString();
        response.success = false;
        response.errorMessage = "Failed to parse API response.";
        callback(response);
        return;
    }

    // Parse the response
    response = parseResponse(responseDoc);

    if (response.success) {
        LOGI() << "Successfully received AI response";
        LOGI() << "AI Response text: " << response.responseText;
        LOGD() << "Response text length: " << response.responseText.length() << " characters";
    } else {
        LOGW() << "API returned error: " << response.errorMessage;
    }

    LOGD() << "About to call callback with response (success=" << response.success << ")";
    callback(response);
    LOGD() << "Callback invoked successfully";
}
