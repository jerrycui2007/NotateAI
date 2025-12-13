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
#include "commandexecutor.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include "extensions/internal/scriptengine.h"

#include "log.h"

using namespace mu::notateai;
using namespace muse::extensions;

CommandExecutor::CommandExecutor(const muse::modularity::ContextPtr& iocCtx, QObject* parent)
    : QObject(parent), Injectable(iocCtx)
{
}

ExecutionResult CommandExecutor::execute(const QString& script)
{
    ExecutionResult result;

    LOGI() << "CommandExecutor: Starting script execution";
    emit executionStarted();

    // Check if a score is open
    auto notation = m_globalContext()->currentNotation();
    if (!notation) {
        result.success = false;
        result.errorMessage = "No score is currently open. Please open a score first.";
        LOGW() << "CommandExecutor:" << result.errorMessage;
        emit executionCompleted(result);
        return result;
    }

    LOGI() << "CommandExecutor: Evaluating script...";
    LOGD() << "Script content:\n" << script;

    // Write script to temp file and execute using ScriptEngine with API v1
    // API v1 provides: curScore, cmd(), newElement(), Cursor, Element enums, etc.
    QString tempPath = QDir::tempPath() + "/notateai_command.js";
    QFile tempFile(tempPath);

    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = "Failed to create temporary script file";
        LOGE() << "CommandExecutor:" << result.errorMessage;
        emit executionCompleted(result);
        return result;
    }

    QTextStream out(&tempFile);
    out << script;
    tempFile.close();

    // Create ScriptEngine with API v1 and evaluate
    ScriptEngine engine(iocContext(), 1 /* apiversion */);
    engine.setScriptPath(tempPath);

    muse::Ret evalRet = engine.evaluate();

    // Clean up temp file
    QFile::remove(tempPath);

    if (!evalRet) {
        result.success = false;
        result.errorMessage = QString::fromStdString(evalRet.text());

        // The error message from ScriptEngine already includes line info in the text
        // No need to extract separately

        LOGW() << "CommandExecutor: Script error -" << result.errorMessage;
    } else {
        result.success = true;
        LOGI() << "CommandExecutor: Script executed successfully";
    }

    emit executionCompleted(result);
    return result;
}
