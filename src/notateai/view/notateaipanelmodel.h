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
#ifndef MU_NOTATEAI_NOTATEAIPANELMODEL_H
#define MU_NOTATEAI_NOTATEAIPANELMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../inotateaiconfiguration.h"
#include "../internal/geminiservice.h"

namespace mu::notateai {
class NotateAIPanelModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool resendScoreData READ resendScoreData WRITE setResendScoreData NOTIFY resendScoreDataChanged)

    muse::Inject<INotateAIConfiguration> configuration = { this };

public:
    explicit NotateAIPanelModel(QObject* parent = nullptr);

    bool isLoading() const;
    bool resendScoreData() const;
    void setResendScoreData(bool resend);

    Q_INVOKABLE void sendMessage(const QString& message);
    Q_INVOKABLE void clearConversation();

signals:
    void messageReceived(QString aiResponse);
    void errorOccurred(QString errorMessage);
    void isLoadingChanged();
    void resendScoreDataChanged();

private:
    void handleGeminiResponse(const GeminiService::GeminiResponse& response);

    GeminiService* m_geminiService = nullptr;
    bool m_isLoading = false;
    bool m_resendScoreData = false;
};
}

#endif // MU_NOTATEAI_NOTATEAIPANELMODEL_H
