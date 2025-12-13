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
#ifndef MU_NOTATEAI_COMMANDPARSER_H
#define MU_NOTATEAI_COMMANDPARSER_H

#include <QString>
#include <QList>

namespace mu::notateai {

struct ParsedCommand {
    QString code;       // The JavaScript code to execute
    int startPos = 0;   // Position in original text where code block starts
    int endPos = 0;     // Position in original text where code block ends
};

class CommandParser
{
public:
    // Extract all notateai code blocks from the response text
    static QList<ParsedCommand> extractCommands(const QString& responseText);

    // Check if the response contains any executable commands
    static bool hasCommands(const QString& responseText);
};

}

#endif // MU_NOTATEAI_COMMANDPARSER_H
