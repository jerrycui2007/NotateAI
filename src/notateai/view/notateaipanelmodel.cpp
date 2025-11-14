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

#include <QThread>
#include <QCoreApplication>
#include <QMetaObject>

#include "log.h"

using namespace mu::notateai;

NotateAIPanelModel::NotateAIPanelModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    m_geminiService = new GeminiService(iocContext());

    // Connect to the GeminiService's responseReceived signal
    // Qt::QueuedConnection ensures the slot is called on this object's thread (main thread)
    connect(m_geminiService, &GeminiService::responseReceived,
            this, &NotateAIPanelModel::handleGeminiResponse,
            Qt::QueuedConnection);

    LOGI() << "NotateAIPanelModel: Connected to GeminiService responseReceived signal";
}

bool NotateAIPanelModel::isLoading() const
{
    return m_isLoading;
}

void NotateAIPanelModel::sendMessage(const QString& message)
{
    LOGI() << "========================================";
    LOGI() << "NotateAIPanelModel::sendMessage called with message: " << message;

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

    LOGI() << "Sending message to Gemini service (via Qt signal/slot)...";

    // Send message to Gemini service
    // The response will be received via the responseReceived signal connection
    m_geminiService->sendMessage(message);

    LOGI() << "Message sent to GeminiService";
    LOGI() << "========================================";
}

void NotateAIPanelModel::handleGeminiResponse(const GeminiService::GeminiResponse& response)
{
    LOGI() << "!!!!! handleGeminiResponse CALLED !!!!!";
    LOGI() << "handleGeminiResponse called on thread: " << QThread::currentThread();
    LOGI() << "Main thread is: " << QCoreApplication::instance()->thread();
    LOGI() << "This object's thread is: " << this->thread();

    // Qt::QueuedConnection in the connect() call ensures we're already on the main thread
    m_isLoading = false;
    emit isLoadingChanged();

    if (response.success) {
        LOGI() << "Successfully received AI response, emitting to QML: " << response.responseText;
        emit messageReceived(response.responseText);
        LOGI() << "messageReceived signal emitted to QML";
    } else {
        LOGW() << "Error from Gemini service: " << response.errorMessage;
        emit errorOccurred(response.errorMessage);
        LOGI() << "errorOccurred signal emitted to QML";
    }

    LOGI() << "handleGeminiResponse finished";
}
