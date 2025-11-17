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
#ifndef MU_NOTATEAI_GEMINISERVICE_H
#define MU_NOTATEAI_GEMINISERVICE_H

#include <QString>
#include <QJsonDocument>
#include <QObject>

#include "modularity/ioc.h"
#include "network/inetworkmanagercreator.h"
#include "../inotateaiconfiguration.h"
#include "async/channel.h"
#include "types/ret.h"
#include "types/retval.h"
#include "context/iglobalcontext.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace mu::notateai {
class GeminiService : public QObject, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<muse::network::INetworkManagerCreator> networkManagerCreator = { this };
    muse::Inject<INotateAIConfiguration> configuration = { this };
    muse::Inject<mu::context::IGlobalContext> m_globalContext = { this };

public:
    GeminiService(const muse::modularity::ContextPtr& iocCtx)
        : QObject(nullptr), Injectable(iocCtx) {}

    struct GeminiResponse {
        bool success = false;
        QString responseText;
        QString errorMessage;
    };

    void sendMessage(const QString& userMessage);

signals:
    void responseReceived(const GeminiService::GeminiResponse& response);

private:
    QString extractScoreDataAsMusicXML() const;
    void th_sendMessage(const QString& userMessage, std::function<void(GeminiResponse)> callback) const;
    void th_sendMessageDirect(const QString& userMessage, std::function<void(GeminiResponse)> callback) const;
    QByteArray buildRequestJson(const QString& userMessage) const;
    GeminiResponse parseResponse(const QJsonDocument& responseDoc) const;
};
}

#endif // MU_NOTATEAI_GEMINISERVICE_H
