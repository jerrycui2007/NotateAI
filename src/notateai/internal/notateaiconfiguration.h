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
#ifndef MU_NOTATEAI_NOTATEAICONFIGURATION_H
#define MU_NOTATEAI_NOTATEAICONFIGURATION_H

#include "../inotateaiconfiguration.h"

#include "modularity/ioc.h"

namespace mu::notateai {
class NotateAIConfiguration : public INotateAIConfiguration, public muse::Injectable
{
public:
    NotateAIConfiguration(const muse::modularity::ContextPtr& iocCtx);

    void init();

    QString geminiApiKey() const override;
    void setGeminiApiKey(const QString& key) override;
};
}

#endif // MU_NOTATEAI_NOTATEAICONFIGURATION_H
