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

// Base system prompt - defines the AI's role and behavior (sent every message)
static const QString BASE_SYSTEM_PROMPT = R"(You are a helpful AI assistant integrated into NotateAI, a music notation software based on MuseScore 4.
You help users with questions about music theory, notation, and using the software. Your role is like an AI coding assistant but for music notation.
If the user sends an off-topic message, politely inform them that you can only assist with music notation related queries.

Your expertise includes:
- Music theory (harmony, counterpoint, form, analysis)
- Music notation and engraving
- Composition and arranging
- Using the NotateAI software and its API

When responding:
- Be clear, concise, and educational
- Use musical examples when helpful
- Be encouraging and supportive
- If you don't know something, admit it rather than making up information
)"
;

// API documentation - sent only on first message of conversation
static const QString API_DOCUMENTATION = R"(
## MuseScore 4 API Reference

You can generate commands to modify the user's score. When the user asks you to make changes to their score, you should respond with the appropriate API calls.

### Adding Notes via Cursor

```javascript
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);  // Navigate to start
cursor.setDuration(1, 4);           // Set quarter note duration
curScore.startCmd("Add Notes");
cursor.addNote(60);                 // Add Middle C (MIDI pitch)
cursor.addNote(62);                 // Add D
curScore.endCmd();
```

### Duration Values (setDuration(z, n) = z/n fraction)
- Whole: (1,1), Half: (1,2), Quarter: (1,4), Eighth: (1,8), 16th: (1,16)
- Dotted quarter: (3,8), Dotted half: (3,4), Dotted eighth: (3,16)

### MIDI Pitch Reference
| Note | Oct 3 | Oct 4 | Oct 5 |
|------|-------|-------|-------|
| C    | 48    | 60    | 72    |
| D    | 50    | 62    | 74    |
| E    | 52    | 64    | 76    |
| F    | 53    | 65    | 77    |
| G    | 55    | 67    | 79    |
| A    | 57    | 69    | 81    |
| B    | 59    | 71    | 83    |

Middle C (C4) = 60, Concert A (A4) = 69

### Building Chords
```javascript
cursor.addNote(60, false);  // C (new chord)
cursor.addNote(64, true);   // Add E to chord
cursor.addNote(67, true);   // Add G to chord (C major)
```

### Common Actions via cmd()
```javascript
cmd("note-input");           // Toggle note input mode
cmd("note-c");               // Enter C
cmd("rest");                 // Enter rest
cmd("pitch-up");             // Pitch up semitone
cmd("pitch-down");           // Pitch down semitone
cmd("pitch-up-octave");      // Up octave
cmd("pitch-down-octave");    // Down octave
cmd("tie");                  // Add tie
cmd("add-slur");             // Add slur
cmd("triplet");              // Enter triplet
cmd("pad-dot");              // Toggle dot
cmd("flat");                 // Add flat
cmd("sharp");                // Add sharp
cmd("natural");              // Add natural
```

### Navigation
```javascript
cursor.next();               // Next segment
cursor.nextMeasure();        // Next measure
cursor.rewind(Cursor.SCORE_START);      // Go to start
cursor.rewind(Cursor.SELECTION_START);  // Go to selection start
```

### Adding Elements
```javascript
var text = newElement(Element.STAFF_TEXT);
text.text = "pizz.";
cursor.add(text);
```

### Score Operations
```javascript
curScore.appendMeasures(4);  // Add 4 measures
curScore.startCmd("Action Name");  // Start undo block
curScore.endCmd();                  // End undo block
```

### Cursor Properties
- `cursor.track` - Current track (staffIdx * 4 + voice)
- `cursor.staffIdx` - Staff number
- `cursor.voice` - Voice (0-3)
- `cursor.element` - Element at cursor
- `cursor.measure` - Current measure
)"
;

void GeminiService::sendMessage(const QString& userMessage, bool includeScoreData)
{
    LOGI() << "GeminiService::sendMessage called with message: " << userMessage;
    LOGI() << "Include score data: " << includeScoreData;

    // Store user message to add to history after successful response
    m_pendingUserMessage = userMessage;

    if (includeScoreData) {
        // STEP 3 VERIFICATION: Test score data extraction
        QString scoreData = extractScoreDataAsMusicXML();
        LOGI() << "=== SCORE DATA EXTRACTION TEST ===";
        LOGI() << "Score data length:" << scoreData.length();
        LOGI() << "First 500 chars:" << scoreData.left(500);
        LOGI() << "===================================";
    }

    auto callback = [this](GeminiResponse response) {
        LOGI() << "Callback lambda invoked! Success: " << response.success;
        LOGI() << "Callback: About to emit responseReceived signal";

        // Add to conversation history if successful
        if (response.success && !m_pendingUserMessage.isEmpty()) {
            // Add user message
            ConversationTurn userTurn;
            userTurn.role = "user";
            userTurn.text = m_pendingUserMessage;
            m_conversationHistory.append(userTurn);

            // Add AI response
            ConversationTurn modelTurn;
            modelTurn.role = "model";
            modelTurn.text = response.responseText;
            m_conversationHistory.append(modelTurn);

            LOGI() << "Conversation history updated, now has" << m_conversationHistory.size() << "turns";
            m_pendingUserMessage.clear();
        }

        // Emit the signal - Qt will handle cross-thread communication automatically
        emit responseReceived(response);

        LOGI() << "Callback: responseReceived signal emitted";
    };

    LOGI() << "Starting concurrent task...";
    // Use lambda to capture all parameters since Concurrent::run only supports up to 2 params
    Concurrent::run([this, userMessage, includeScoreData, callback]() {
        th_sendMessageDirect(userMessage, includeScoreData, callback);
    });
    LOGI() << "Concurrent task started";
}

void GeminiService::clearHistory()
{
    m_conversationHistory.clear();
    m_pendingUserMessage.clear();
    LOGI() << "Conversation history cleared";
}

void GeminiService::th_sendMessage(const QString& userMessage, bool includeScoreData, std::function<void(GeminiResponse)> callback)
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
    QByteArray requestJson = buildRequestJson(userMessage, includeScoreData);

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

QByteArray GeminiService::buildRequestJson(const QString& userMessage, bool includeScoreData) const
{
    QJsonObject requestObj;

    // Build system prompt - include API docs only on first message (empty conversation history)
    bool isFirstMessage = m_conversationHistory.isEmpty();
    QString systemPrompt = BASE_SYSTEM_PROMPT;

    if (isFirstMessage) {
        // Include full API documentation on first message
        systemPrompt += API_DOCUMENTATION;
        LOGI() << "=== FIRST MESSAGE - Including API documentation ===";
    } else {
        LOGI() << "=== SUBSEQUENT MESSAGE - API docs already in context ===";
    }

    if (includeScoreData) {
        // Extract current score data only if requested
        QString scoreData = extractScoreDataAsMusicXML();

        if (!scoreData.isEmpty() &&
            scoreData != "No score currently open" &&
            scoreData != "Score data unavailable" &&
            !scoreData.startsWith("Error exporting score")) {
            systemPrompt += "\n\n## Current Score Context\n\n";
            systemPrompt += "The user is currently working on the following musical score (in MusicXML format):\n\n";
            systemPrompt += "```xml\n";
            systemPrompt += scoreData;
            systemPrompt += "\n```\n\n";
            systemPrompt += "Please use this score data to provide contextually relevant responses about the user's music.";

            LOGI() << "=== SCORE DATA INCLUDED ===";
            LOGI() << "Score data length:" << scoreData.length() << "characters";
            LOGI() << "===========================";
        }
    } else {
        LOGI() << "Score data NOT included in this request";
    }

    // System instruction
    QJsonObject systemInstructionObj;
    QJsonArray systemPartsArray;
    QJsonObject systemPartObj;
    systemPartObj["text"] = systemPrompt;
    systemPartsArray.append(systemPartObj);
    systemInstructionObj["parts"] = systemPartsArray;
    requestObj["systemInstruction"] = systemInstructionObj;

    // Build the contents array with conversation history
    QJsonArray contentsArray;

    // Add previous conversation turns
    for (const ConversationTurn& turn : m_conversationHistory) {
        QJsonObject turnObj;
        turnObj["role"] = turn.role;

        QJsonArray partsArray;
        QJsonObject partObj;
        partObj["text"] = turn.text;
        partsArray.append(partObj);

        turnObj["parts"] = partsArray;
        contentsArray.append(turnObj);
    }

    // Add current user message
    QJsonObject currentMessageObj;
    currentMessageObj["role"] = "user";

    QJsonArray currentPartsArray;
    QJsonObject currentPartObj;
    currentPartObj["text"] = userMessage;
    currentPartsArray.append(currentPartObj);

    currentMessageObj["parts"] = currentPartsArray;
    contentsArray.append(currentMessageObj);

    requestObj["contents"] = contentsArray;

    QJsonDocument doc(requestObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    // Log request details
    LOGI() << "=== REQUEST JSON DETAILS ===";
    LOGI() << "First message (includes API docs):" << isFirstMessage;
    LOGI() << "Total request size:" << jsonData.size() << "bytes";
    LOGI() << "Conversation history turns:" << m_conversationHistory.size();
    LOGI() << "Include score data:" << includeScoreData;
    LOGI() << "=============================";

    return jsonData;
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

void GeminiService::th_sendMessageDirect(const QString& userMessage, bool includeScoreData, std::function<void(GeminiResponse)> callback)
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
    QByteArray requestJson = buildRequestJson(userMessage, includeScoreData);

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
