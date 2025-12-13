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

You can generate commands to modify the user's score. When the user asks you to make changes, you MUST provide REAL EXECUTABLE JAVASCRIPT CODE in a ```notateai code block.

CRITICAL RULES:
1. Use ```notateai (not ```javascript) - this triggers the Execute button
2. Write ACTUAL JavaScript code, NOT shorthand or pseudocode
3. Use MIDI pitch numbers (60 = Middle C, 62 = D, 64 = E, etc.)
4. Always wrap modifications in curScore.startCmd()/endCmd() for undo support
5. DO NOT invent methods - ONLY use the methods documented below
6. There is NO goToMeasure() or goTo() method - use rewind() and nextMeasure()

### MIDI Pitch Values (MUST use these numbers, not note names)
C3=48, D3=50, E3=52, F3=53, G3=55, A3=57, B3=59
C4=60, D4=62, E4=64, F4=65, G4=67, A4=69, B4=71
C5=72, D5=74, E5=76, F5=77, G5=79, A5=81, B5=83
For flats: Eb4=63, Ab4=68, Bb4=70
For sharps: C#4=61, F#4=66, G#4=68

### Complete Example - Adding Notes to Measure 1

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.staffIdx = 0;
cursor.voice = 0;
cursor.setDuration(1, 4);  // Quarter notes
curScore.startCmd("Add Notes");
cursor.addNote(60);  // C4
cursor.addNote(62);  // D4
cursor.addNote(64);  // E4
cursor.addNote(65);  // F4
curScore.endCmd();
```

### Navigating to a Specific Measure

There is NO goToMeasure() method. To navigate to measure N, use nextMeasure():

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
// To go to measure 3 (0-indexed, so this is the 3rd measure):
cursor.nextMeasure();  // Now at measure 1
cursor.nextMeasure();  // Now at measure 2
// Now cursor is at measure 3
```

OR use a loop:

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
var targetMeasure = 2;  // 0-indexed
for (var i = 0; i < targetMeasure; i++) {
    cursor.nextMeasure();
}
// Now at measure 2
```

### Clearing a Measure's Content

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
// Navigate to the measure first
for (var i = 0; i < 1; i++) {  // Go to measure 1
    cursor.nextMeasure();
}
curScore.startCmd("Clear Measure");
// Delete all elements in this measure
while (cursor.segment && cursor.measure.is(someMeasure)) {
    var el = cursor.element;
    if (el) {
        removeElement(el);
    }
    cursor.next();
}
curScore.endCmd();
```

Note: Clearing measures is complex. It's often easier to add rests to overwrite content.

### Duration Values - setDuration(numerator, denominator)
- Whole note: cursor.setDuration(1, 1)
- Half note: cursor.setDuration(1, 2)
- Quarter note: cursor.setDuration(1, 4)
- Eighth note: cursor.setDuration(1, 8)
- 16th note: cursor.setDuration(1, 16)
- Dotted quarter: cursor.setDuration(3, 8)  // 1/4 * 3/2 = 3/8
- Dotted half: cursor.setDuration(3, 4)     // 1/2 * 3/2 = 3/4

### Building Chords (multiple notes on same beat)

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.setDuration(1, 1);  // Whole note
curScore.startCmd("Add Chord");
cursor.addNote(60, false);  // C4 - creates new chord
cursor.addNote(64, true);   // E4 - adds to chord
cursor.addNote(67, true);   // G4 - adds to chord (C major triad)
curScore.endCmd();
```

### Adding Rests

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.setDuration(1, 4);
curScore.startCmd("Add Rest");
cursor.addRest();
curScore.endCmd();
```

### Complete Example: Add Chords to Multiple Measures

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.staffIdx = 0;
cursor.voice = 0;

curScore.startCmd("Add Chord Progression");

// Measure 1 - C major chord (whole note)
cursor.setDuration(1, 1);
cursor.addNote(60, false);  // C
cursor.addNote(64, true);   // E
cursor.addNote(67, true);   // G

// Move to measure 2
cursor.nextMeasure();

// Measure 2 - F major chord (whole note)
cursor.setDuration(1, 1);
cursor.addNote(65, false);  // F
cursor.addNote(69, true);   // A
cursor.addNote(72, true);   // C

// Move to measure 3
cursor.nextMeasure();

// Measure 3 - G major chord (whole note)
cursor.setDuration(1, 1);
cursor.addNote(67, false);  // G
cursor.addNote(71, true);   // B
cursor.addNote(74, true);   // D

curScore.endCmd();
```

### ALL Available Cursor Methods (DO NOT use methods not listed here):
- cursor.rewind(Cursor.SCORE_START) - Go to start of score
- cursor.rewind(Cursor.SELECTION_START) - Go to selection start
- cursor.rewind(Cursor.SELECTION_END) - Go to selection end
- cursor.next() - Move to next segment (returns false at end)
- cursor.nextMeasure() - Move to next measure (returns false at end)
- cursor.prev() - Move to previous segment
- cursor.setDuration(numerator, denominator) - Set duration for notes/rests
- cursor.addNote(pitch, addToChord) - Add note (pitch is MIDI number 0-127)
- cursor.addRest() - Add rest with current duration
- cursor.add(element) - Add an element

### ALL Available Cursor Properties:
- cursor.staffIdx - Staff number (0-indexed, read/write)
- cursor.voice - Voice 0-3 (read/write)
- cursor.tick - Current tick position (read only)
- cursor.element - Current element (read only)
- cursor.segment - Current segment (read only)
- cursor.measure - Current measure (read only)

### Score Methods:
- curScore.startCmd("Command Name") - Start undo block (REQUIRED before modifications)
- curScore.endCmd() - End undo block (REQUIRED after modifications)
- curScore.newCursor() - Create new cursor
- curScore.nmeasures - Number of measures (read only)
- curScore.appendMeasures(n) - Add n measures to end of score
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
