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
#include "notateaimodule.h"

#include "log.h"

#include "modularity/ioc.h"

using namespace mu::notateai;
using namespace muse;
using namespace muse::modularity;

std::string NotateAIModule::moduleName() const
{
    return "notateai";
}

void NotateAIModule::registerExports()
{
    // Future: Configuration and service exports will be registered here
}

void NotateAIModule::resolveImports()
{
    // Future: UI actions and interactive URI registrations will be added here
}

void NotateAIModule::registerResources()
{
    // Future: QRC resources will be initialized here when we add QML files
}

void NotateAIModule::registerUiTypes()
{
    // Future: QML types will be registered here (e.g., NotateAIPanelModel)
}

void NotateAIModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    // Future: Initialize configuration and services here
    LOGI() << "NotateAI module initialized";
}

void NotateAIModule::onAllInited(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    // Future: Additional setup after all modules are initialized
}

void NotateAIModule::onDeinit()
{
    // Future: Cleanup resources here
}
