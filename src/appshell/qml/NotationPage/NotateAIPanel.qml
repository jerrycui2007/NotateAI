/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    // Chat message model
    ListModel {
        id: chatMessagesModel
    }

    // Function to add a message
    function addMessage(text, isUser) {
        chatMessagesModel.append({
            "messageText": text,
            "isUserMessage": isUser
        })
        // Scroll to bottom after adding message
        messagesView.positionViewAtEnd()
    }

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor

        Column {
            anchors.fill: parent

            // Chat messages area
            Rectangle {
                id: messagesArea
                width: parent.width
                height: parent.height - inputArea.height
                color: ui.theme.backgroundPrimaryColor

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true

                    ListView {
                        id: messagesView
                        width: parent.width
                        spacing: 8
                        model: chatMessagesModel

                        delegate: Rectangle {
                            width: messagesView.width
                            height: messageTextItem.implicitHeight + 16
                            color: model.isUserMessage ? ui.theme.backgroundSecondaryColor : ui.theme.backgroundTertiaryColor
                            radius: 8

                            Text {
                                id: messageTextItem
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.margins: 8
                                text: model.messageText
                                wrapMode: Text.Wrap
                                font: ui.theme.bodyFont
                                color: ui.theme.fontPrimaryColor
                            }
                        }
                    }
                }
            }

            // Input area at the bottom
            Rectangle {
                id: inputArea
                width: parent.width
                height: Math.min(200, Math.max(60, textArea.contentHeight + 48))
                color: ui.theme.backgroundSecondaryColor

                Row {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8
                    anchors.bottomMargin: 8

                    // Text input field
                    Rectangle {
                        width: parent.width - sendButton.width - parent.spacing
                        height: parent.height
                        color: ui.theme.textFieldColor
                        border.color: ui.theme.strokeColor
                        border.width: 1
                        radius: 4

                        ScrollView {
                            anchors.fill: parent
                            anchors.margins: 8
                            clip: true

                            TextArea {
                                id: textArea
                                width: parent.width
                                wrapMode: TextArea.Wrap
                                font: ui.theme.bodyFont
                                color: ui.theme.fontPrimaryColor
                                background: null
                                selectByMouse: true

                                placeholderText: qsTrc("appshell", "Type a message...")
                                placeholderTextColor: ui.theme.fontSecondaryColor
                            }
                        }
                    }

                    // Send button (up arrow)
                    FlatButton {
                        id: sendButton
                        width: 44
                        height: 44
                        anchors.bottom: parent.bottom
                        icon: IconCode.ARROW_UP

                        onClicked: {
                            var message = textArea.text.trim()
                            if (message.length > 0) {
                                // Add user message
                                root.addMessage(message, true)

                                // Clear text box
                                textArea.text = ""

                                // TODO: Backend communication for received messages will be added here
                            }
                        }
                    }
                }
            }
        }
    }
}
