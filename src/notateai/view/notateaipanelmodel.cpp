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
#include "notateaipanelmodel.h"

#include "log.h"

using namespace mu::notateai;

NotateAIPanelModel::NotateAIPanelModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    m_geminiService = new GeminiService(iocContext());
}

bool NotateAIPanelModel::isLoading() const
{
    return m_isLoading;
}

void NotateAIPanelModel::sendMessage(const QString& message)
{
    LOGD() << "NotateAIPanelModel::sendMessage called with message: " << message;

    if (m_isLoading) {
        LOGW() << "Already loading, ignoring new request";
        return;
    }

    // Check if API key is configured
    QString apiKey = configuration()->geminiApiKey();
    if (apiKey.isEmpty()) {
        LOGW() << "No API key configured";
        emit errorOccurred("No API key configured. Please add one in Preferences.");
        return;
    }

    m_isLoading = true;
    emit isLoadingChanged();

    LOGD() << "Sending message to Gemini service...";

    // Send message to Gemini service
    muse::async::Channel<GeminiService::GeminiResponse> responseChannel = m_geminiService->sendMessage(message);

    responseChannel.onReceive(this, [this](const GeminiService::GeminiResponse& response) {
        LOGD() << "Received response from Gemini service";
        handleGeminiResponse(response);
    });
}

void NotateAIPanelModel::handleGeminiResponse(const GeminiService::GeminiResponse& response)
{
    m_isLoading = false;
    emit isLoadingChanged();

    if (response.success) {
        LOGD() << "Successfully received AI response: " << response.responseText;
        emit messageReceived(response.responseText);
    } else {
        LOGW() << "Error from Gemini service: " << response.errorMessage;
        emit errorOccurred(response.errorMessage);
    }
}
