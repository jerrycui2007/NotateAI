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
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotateAI 1.0

PreferencesPage {
    id: root

    NotateAIPreferencesModel {
        id: preferencesModel
    }

    Component.onCompleted: {
        preferencesModel.load()
    }

    Column {
        width: parent.width
        spacing: root.sectionsSpacing

        StyledTextLabel {
            width: parent.width
            text: qsTrc("notateai/preferences", "Google Gemini API Configuration")
            font: ui.theme.largeBodyBoldFont
        }

        SeparatorLine { }

        Column {
            width: parent.width
            spacing: 16

            StyledTextLabel {
                width: parent.width
                text: qsTrc("notateai/preferences", "API Key")
                font: ui.theme.bodyBoldFont
            }

            TextInputField {
                id: apiKeyField
                width: parent.width
                currentText: preferencesModel.geminiApiKey

                inputField.echoMode: TextInput.Password
                hint: qsTrc("notateai/preferences", "Enter your Gemini API key")

                navigation.name: "ApiKeyField"
                navigation.panel: root.navigation
                navigation.column: 1

                onTextChanged: function(newValue) {
                    preferencesModel.geminiApiKey = newValue
                }
            }

            Row {
                width: parent.width
                spacing: 4

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("notateai/preferences", "Don't have an API key?")
                    font: ui.theme.bodyFont
                }

                FlatButton {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("notateai/preferences", "Get one from Google AI Studio")

                    navigation.name: "GetApiKeyButton"
                    navigation.panel: root.navigation
                    navigation.column: 2

                    onClicked: {
                        Qt.openUrlExternally(preferencesModel.geminiApiKeyUrl())
                    }
                }
            }

            SeparatorLine { }

            StyledTextLabel {
                width: parent.width
                text: qsTrc("notateai/preferences", "How to get your API key:")
                font: ui.theme.bodyBoldFont
            }

            Column {
                width: parent.width
                spacing: 8

                StyledTextLabel {
                    width: parent.width
                    text: "1. " + qsTrc("notateai/preferences", "Visit Google AI Studio (click the button above)")
                    font: ui.theme.bodyFont
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    width: parent.width
                    text: "2. " + qsTrc("notateai/preferences", "Sign in with your Google account")
                    font: ui.theme.bodyFont
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    width: parent.width
                    text: "3. " + qsTrc("notateai/preferences", "Click 'Create API Key' or 'Get API Key'")
                    font: ui.theme.bodyFont
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    width: parent.width
                    text: "4. " + qsTrc("notateai/preferences", "Copy the key and paste it in the field above")
                    font: ui.theme.bodyFont
                    wrapMode: Text.WordWrap
                }
            }

            SeparatorLine { }

            StyledTextLabel {
                width: parent.width
                text: qsTrc("notateai/preferences", "Note: Your API key is stored locally and securely. It will never be shared with anyone except Google's Gemini API service.")
                font: ui.theme.bodyFont
                opacity: 0.7
                wrapMode: Text.WordWrap
            }
        }
    }
}
