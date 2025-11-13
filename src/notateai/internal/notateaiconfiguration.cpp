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
#include "notateaiconfiguration.h"

#include "settings.h"

using namespace mu::notateai;
using namespace muse;

static const std::string module_name("notateai");
static const Settings::Key GEMINI_API_KEY(module_name, "notateai/geminiApiKey");

NotateAIConfiguration::NotateAIConfiguration(const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx)
{
}

void NotateAIConfiguration::init()
{
    // Initialize with empty API key if not set
    if (settings()->value(GEMINI_API_KEY).isNull()) {
        settings()->setSharedValue(GEMINI_API_KEY, Val(QString()));
    }
}

QString NotateAIConfiguration::geminiApiKey() const
{
    return settings()->value(GEMINI_API_KEY).toQString();
}

void NotateAIConfiguration::setGeminiApiKey(const QString& key)
{
    settings()->setSharedValue(GEMINI_API_KEY, Val(key));
}
