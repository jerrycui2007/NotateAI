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
#ifndef MU_NOTATEAI_COMMANDEXECUTOR_H
#define MU_NOTATEAI_COMMANDEXECUTOR_H

#include <QObject>
#include <QString>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::notateai {

struct ExecutionResult {
    bool success = false;
    QString errorMessage;
    int errorLine = -1;
};

class CommandExecutor : public QObject, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<mu::context::IGlobalContext> m_globalContext = { this };

public:
    explicit CommandExecutor(const muse::modularity::ContextPtr& iocCtx, QObject* parent = nullptr);

    // Execute a single script block
    ExecutionResult execute(const QString& script);

signals:
    void executionStarted();
    void executionCompleted(const ExecutionResult& result);
};

}

#endif // MU_NOTATEAI_COMMANDEXECUTOR_H
