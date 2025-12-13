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
#include "commandparser.h"

#include <QRegularExpression>

#include "log.h"

using namespace mu::notateai;

QList<ParsedCommand> CommandParser::extractCommands(const QString& responseText)
{
    QList<ParsedCommand> commands;

    LOGI() << "CommandParser: Parsing response text, length:" << responseText.length();

    // Match ```notateai code blocks
    // Pattern: ```notateai followed by optional whitespace/newline, then capture everything until closing ```
    // The closing ``` may or may not have a newline before it
    static const QRegularExpression regex(
        R"(```notateai\s*([\s\S]*?)```)",
        QRegularExpression::MultilineOption
    );

    QRegularExpressionMatchIterator it = regex.globalMatch(responseText);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();

        ParsedCommand cmd;
        cmd.code = match.captured(1).trimmed();
        cmd.startPos = match.capturedStart(0);
        cmd.endPos = match.capturedEnd(0);

        LOGI() << "CommandParser: Matched block from" << cmd.startPos << "to" << cmd.endPos;
        LOGI() << "CommandParser: Code content:" << cmd.code.left(100) << "...";

        if (!cmd.code.isEmpty()) {
            commands.append(cmd);
            LOGI() << "CommandParser: Found notateai code block, length:" << cmd.code.length();
        }
    }

    LOGI() << "CommandParser: Extracted" << commands.size() << "command(s) from response";

    // Debug: if no commands found, check if there's any code block pattern
    if (commands.isEmpty()) {
        if (responseText.contains("```notateai")) {
            LOGW() << "CommandParser: Found '```notateai' text but regex didn't match - check formatting";
        }
        if (responseText.contains("```javascript")) {
            LOGW() << "CommandParser: Found '```javascript' - AI should use '```notateai' instead";
        }
    }

    return commands;
}

bool CommandParser::hasCommands(const QString& responseText)
{
    static const QRegularExpression regex(
        R"(```notateai\s*[\s\S]*?```)",
        QRegularExpression::MultilineOption
    );

    return regex.match(responseText).hasMatch();
}
