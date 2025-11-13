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

#include "global/concurrency/concurrent.h"
#include "log.h"

using namespace mu::notateai;
using namespace muse;
using namespace muse::network;

static const QString GEMINI_API_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent";

async::Channel<GeminiService::GeminiResponse> GeminiService::sendMessage(const QString& userMessage)
{
    async::Channel<GeminiResponse> responseChannel;

    auto callback = [responseChannel](GeminiResponse response) mutable {
        responseChannel.send(response);
        responseChannel.close();
    };

    Concurrent::run(this, &GeminiService::th_sendMessage, userMessage, callback);

    return responseChannel;
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

QByteArray GeminiService::buildRequestJson(const QString& userMessage) const
{
    QJsonObject requestObj;

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
