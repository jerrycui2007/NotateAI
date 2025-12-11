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
import MuseScore.NotateAI 1.0

Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    // NotateAI Panel Model
    NotateAIPanelModel {
        id: panelModel

        onMessageReceived: function(aiResponse) {
            console.log("QML: onMessageReceived signal received, response length:", aiResponse.length)
            console.log("QML: AI Response text:", aiResponse)
            root.addMessage(aiResponse, false)
            console.log("QML: Message added to chat")
        }

        onErrorOccurred: function(errorMessage) {
            console.log("QML: onErrorOccurred signal received:", errorMessage)
            root.addMessage("Error: " + errorMessage, false)
        }
    }

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
                            color: model.isUserMessage ? ui.theme.backgroundSecondaryColor : "transparent"
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

                        footer: Item {
                            width: messagesView.width
                            height: panelModel.isLoading ? 40 : 0
                            visible: panelModel.isLoading

                            Row {
                                anchors.centerIn: parent
                                spacing: 4

                                Repeater {
                                    model: 3
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: ui.theme.fontSecondaryColor
                                        opacity: 0.3

                                        SequentialAnimation on opacity {
                                            running: panelModel.isLoading
                                            loops: Animation.Infinite
                                            PauseAnimation { duration: index * 200 }
                                            NumberAnimation { from: 0.3; to: 1.0; duration: 600 }
                                            NumberAnimation { from: 1.0; to: 0.3; duration: 600 }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Input area at the bottom
            Rectangle {
                id: inputArea
                width: parent.width
                height: Math.min(200, Math.max(100, textArea.contentHeight + 88))
                color: ui.theme.backgroundSecondaryColor

                Column {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    // Controls row (checkbox and clear button)
                    Row {
                        width: parent.width
                        spacing: 8

                        CheckBox {
                            id: resendScoreCheckbox
                            text: qsTrc("appshell", "Include score data")
                            checked: panelModel.resendScoreData

                            onClicked: {
                                panelModel.resendScoreData = !panelModel.resendScoreData
                            }
                        }

                        Item {
                            // Spacer
                            width: parent.width - resendScoreCheckbox.width - clearButton.width - parent.spacing * 2
                            height: 1
                        }

                        FlatButton {
                            id: clearButton
                            width: 80
                            height: 28
                            text: qsTrc("appshell", "Clear")
                            enabled: !panelModel.isLoading

                            onClicked: {
                                chatMessagesModel.clear()
                                panelModel.clearConversation()
                            }
                        }
                    }

                    // Text input row
                    Row {
                        width: parent.width
                        height: parent.height - 36  // Account for controls row
                        spacing: 8

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
                            enabled: !panelModel.isLoading && textArea.text.trim().length > 0

                            onClicked: {
                                var message = textArea.text.trim()
                                if (message.length > 0) {
                                    // Add user message
                                    root.addMessage(message, true)

                                    // Clear text box
                                    textArea.text = ""

                                    // Send message to backend
                                    panelModel.sendMessage(message)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
