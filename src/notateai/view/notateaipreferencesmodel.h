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
#ifndef MU_NOTATEAI_NOTATEAIPREFERENCESMODEL_H
#define MU_NOTATEAI_NOTATEAIPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "../inotateaiconfiguration.h"

namespace mu::notateai {
class NotateAIPreferencesModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(QString geminiApiKey READ geminiApiKey WRITE setGeminiApiKey NOTIFY geminiApiKeyChanged)

    muse::Inject<INotateAIConfiguration> configuration = { this };

public:
    explicit NotateAIPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE QString geminiApiKeyUrl() const;

    QString geminiApiKey() const;

public slots:
    void setGeminiApiKey(const QString& key);

signals:
    void geminiApiKeyChanged(const QString& key);

private:
    QString m_geminiApiKey;
};
}

#endif // MU_NOTATEAI_NOTATEAIPREFERENCESMODEL_H
