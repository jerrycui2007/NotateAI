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
#include "geminiservice.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <chrono>
#include <thread>

#include "global/concurrency/concurrent.h"
#include "log.h"
#include "context/iglobalcontext.h"
#include "notation/inotation.h"
#include "notation/inotationelements.h"
#include "engraving/dom/score.h"
#include "engraving/dom/masterscore.h"
#include "importexport/musicxml/internal/musicxml/export/exportmusicxml.h"
#include "io/buffer.h"

using namespace mu::notateai;
using namespace muse;
using namespace muse::network;

static const QString GEMINI_API_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent";

// Base system prompt - defines the AI's role and behavior (sent every message)
static const QString BASE_SYSTEM_PROMPT = R"(You are a helpful AI assistant integrated into NotateAI, a music notation software based on MuseScore 4.
You help users with questions about music theory, notation, and using the software. Your role is like an AI coding assistant but for music notation.
If the user sends an off-topic message, politely inform them that you can only assist with music notation related queries.

Your expertise includes:
- Music theory (harmony, counterpoint, form, analysis)
- Music notation and engraving
- Composition and arranging
- Using the NotateAI software and its API

When responding:
- Be clear, concise, and educational
- Use musical examples when helpful
- Be encouraging and supportive
- If you don't know something, admit it rather than making up information
)"
;

// API documentation - sent only on first message of conversation
static const QString API_DOCUMENTATION = R"(
## MuseScore 4 API Reference

You can generate commands to modify the user's score. When the user asks you to make changes, you MUST provide REAL EXECUTABLE JAVASCRIPT CODE in a ```notateai code block.

CRITICAL RULES:
1. Use ```notateai (not ```javascript) - this triggers the Execute button
2. Write ACTUAL JavaScript code, NOT shorthand or pseudocode
3. Use MIDI pitch numbers (60 = Middle C, 62 = D, 64 = E, etc.)
4. Always wrap modifications in curScore.startCmd()/endCmd() for undo support
5. DO NOT invent methods - ONLY use the methods documented below
6. There is NO goToMeasure() or goTo() method - use rewind() and nextMeasure()
7. CURSOR AUTO-ADVANCES: When you call addNote(pitch, false) to create a new note/chord, the cursor automatically moves forward by the current duration. You do NOT need to call next() between notes!
8. RESPECT USER SELECTION: Always check if the user has selected a region and work within it. Use Cursor.SELECTION_START when available, not just Cursor.SCORE_START.

### MIDI Pitch Values (MUST use these numbers, not note names)
C3=48, D3=50, E3=52, F3=53, G3=55, A3=57, B3=59
C4=60, D4=62, E4=64, F4=65, G4=67, A4=69, B4=71
C5=72, D5=74, E5=76, F5=77, G5=79, A5=81, B5=83
For flats: Eb4=63, Ab4=68, Bb4=70
For sharps: C#4=61, F#4=66, G#4=68

### IMPORTANT: Understanding Cursor Auto-Advance

When you add a note or chord, the cursor AUTOMATICALLY moves forward by the current duration:
- cursor.setDuration(1, 4) sets quarter note duration
- cursor.addNote(60) adds a quarter note AND moves cursor forward by one quarter note
- The NEXT addNote() will be at the next quarter note position

This is why you can add sequential notes without calling cursor.next()!

### Working with User Selections (ALWAYS CHECK FOR THIS!)

CRITICAL: Users may select a specific region of the score. ALWAYS check for selections and respect them!

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SELECTION_START);

if (!cursor.segment) {
    // No selection - user wants to work with whole score
    cursor.rewind(Cursor.SCORE_START);
    api.log.info("No selection, working from start of score");
} else {
    // Selection exists - work within it
    var startTick = cursor.tick;
    var startStaff = cursor.staffIdx;
    cursor.rewind(Cursor.SELECTION_END);
    var endTick = cursor.tick;
    var endStaff = cursor.staffIdx;

    // Reset to selection start
    cursor.rewind(Cursor.SELECTION_START);
    api.log.info("Working within selection from tick " + startTick + " to " + endTick);
}
```

### Complete Example - Adding Sequential Notes (EVERY measure will be filled!)

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.staffIdx = 0;
cursor.voice = 0;
cursor.setDuration(1, 4);  // Quarter notes

curScore.startCmd("Add Notes");
// Each addNote automatically advances the cursor by one quarter note
cursor.addNote(60);  // C4 at beat 1
cursor.addNote(62);  // D4 at beat 2 (cursor auto-advanced!)
cursor.addNote(64);  // E4 at beat 3 (cursor auto-advanced!)
cursor.addNote(65);  // F4 at beat 4 (cursor auto-advanced!)
// These notes fill measure 1 completely with quarter notes
curScore.endCmd();
```

### Navigating to a Specific Measure

WARNING: Measures are 0-indexed! Measure 1 (first measure) = index 0.

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
// cursor is now at measure 0 (the first measure)

// To go to measure 2 (the THIRD measure, 0-indexed):
cursor.nextMeasure();  // Now at measure 1 (second measure)
cursor.nextMeasure();  // Now at measure 2 (third measure)
```

Better approach using a loop to go to a specific measure:

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
var targetMeasureIndex = 2;  // Want to work in measure 3 (0-indexed = 2)

for (var i = 0; i < targetMeasureIndex; i++) {
    if (!cursor.nextMeasure()) {
        api.log.error("Measure " + targetMeasureIndex + " does not exist!");
        break;
    }
}
// Now at measure index 2 (the 3rd measure)
```

### Duration Values - setDuration(numerator, denominator)
- Whole note: cursor.setDuration(1, 1)
- Half note: cursor.setDuration(1, 2)
- Quarter note: cursor.setDuration(1, 4)
- Eighth note: cursor.setDuration(1, 8)
- 16th note: cursor.setDuration(1, 16)
- Dotted quarter: cursor.setDuration(3, 8)  // 1/4 * 3/2 = 3/8
- Dotted half: cursor.setDuration(3, 4)     // 1/2 * 3/2 = 3/4

### Building Chords (multiple notes stacked vertically on same beat)

CRITICAL: To create a chord, ALL notes must be added BEFORE the cursor advances!
- First note: addNote(pitch, false) - creates new chord
- Additional notes: addNote(pitch, true) - adds to the SAME chord
- Next note/chord: addNote(pitch, false) - cursor auto-advances and creates new chord

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.setDuration(1, 1);  // Whole note

curScore.startCmd("Add Chord");
// These three notes form a SINGLE chord (vertical stack)
cursor.addNote(60, false);  // C4 - creates new chord, cursor does NOT advance yet
cursor.addNote(64, true);   // E4 - adds to same chord, cursor does NOT advance yet
cursor.addNote(67, true);   // G4 - adds to same chord (C major triad)
// Now the chord is complete. Next addNote with false will auto-advance cursor.
curScore.endCmd();
```

### Adding Rests

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.setDuration(1, 4);
curScore.startCmd("Add Rest");
cursor.addRest();  // Adds quarter rest and auto-advances cursor
curScore.endCmd();
```

### Complete Example: Add Chords to CONSECUTIVE Measures (NO GAPS!)

CRITICAL: This example adds chords to measures 0, 1, and 2 (first three measures).
After adding a chord with addNote(pitch, false), the cursor automatically advances!

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.staffIdx = 0;
cursor.voice = 0;

curScore.startCmd("Add Chord Progression");

cursor.setDuration(1, 1);  // Whole notes

// Measure 0 - C major chord (whole note)
cursor.addNote(60, false);  // C - creates chord
cursor.addNote(64, true);   // E - adds to chord
cursor.addNote(67, true);   // G - completes C major chord
// Cursor is STILL in measure 0 (whole note chord fills the measure)

// The next addNote with false will auto-advance the cursor by one whole note (one measure)
// So this will be in measure 1:
cursor.addNote(65, false);  // F - cursor auto-advanced to measure 1, creates new chord
cursor.addNote(69, true);   // A - adds to chord
cursor.addNote(72, true);   // C - completes F major chord

// Again, next addNote with false auto-advances to measure 2:
cursor.addNote(67, false);  // G - cursor auto-advanced to measure 2, creates new chord
cursor.addNote(71, true);   // B - adds to chord
cursor.addNote(74, true);   // D - completes G major chord

curScore.endCmd();
```

ALTERNATIVE: If you need explicit control over measure boundaries, use nextMeasure():

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);

curScore.startCmd("Add Chords with Explicit Measure Navigation");
cursor.setDuration(1, 4);  // Quarter notes (4 per measure in 4/4 time)

// Measure 0 - fill with quarter note chords
for (var beat = 0; beat < 4; beat++) {
    cursor.addNote(60, false);
    cursor.addNote(64, true);
    cursor.addNote(67, true);
}
// After 4 quarter notes, cursor has auto-advanced through measure 0

// Move to measure 1 (if not already there, this is a safety check)
cursor.nextMeasure();

// Measure 1 - fill with quarter note chords
for (var beat = 0; beat < 4; beat++) {
    cursor.addNote(65, false);
    cursor.addNote(69, true);
    cursor.addNote(72, true);
}

curScore.endCmd();
```

### Iterating Through All Measures in Score

To process every measure without gaps:

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.voice = 0;

curScore.startCmd("Process All Measures");

var measureCount = 0;
do {
    api.log.info("Processing measure " + measureCount);
    cursor.setDuration(1, 1);  // Whole note
    cursor.addNote(60 + measureCount % 12);  // Different note each measure
    measureCount++;
} while (cursor.nextMeasure());  // Continues until last measure

curScore.endCmd();
```

### ALL Available Cursor Methods (DO NOT use methods not listed here):
- cursor.rewind(Cursor.SCORE_START) - Go to start of score (measure 0)
- cursor.rewind(Cursor.SELECTION_START) - Go to selection start (ALWAYS CHECK THIS FIRST!)
- cursor.rewind(Cursor.SELECTION_END) - Go to selection end
- cursor.next() - Move to next segment within measure (rarely needed due to auto-advance)
- cursor.nextMeasure() - Move to start of next measure (returns false if at end)
- cursor.prev() - Move to previous segment
- cursor.setDuration(numerator, denominator) - Set duration for next notes/rests
- cursor.addNote(pitch, addToChord) - Add note; if addToChord=false (default), cursor auto-advances by current duration
- cursor.addRest() - Add rest with current duration; cursor auto-advances
- cursor.add(element) - Add an element

### ALL Available Cursor Properties:
- cursor.staffIdx - Staff number (0-indexed, read/write)
- cursor.voice - Voice 0-3 (read/write)
- cursor.tick - Current tick position (read only)
- cursor.element - Current element (read only)
- cursor.segment - Current segment (read only, null if no segment)
- cursor.measure - Current measure (read only)

### Score Methods:
- curScore.startCmd("Command Name") - Start undo block (REQUIRED before modifications)
- curScore.endCmd() - End undo block (REQUIRED after modifications)
- curScore.newCursor() - Create new cursor
- curScore.nmeasures - Number of measures (read only)
- curScore.nstaves - Number of staves in score (read only)
- curScore.ntracks - Number of tracks = nstaves * 4 (read only)
- curScore.parts - Array of Part objects (read only)
- curScore.appendMeasures(n) - Add n measures to end of score

### Working with Multiple Instruments/Staves

CRITICAL: Scores often have multiple instruments (e.g., piano, violin, cello in a trio).
Each instrument is a "Part" and each part has one or more "Staves" (e.g., piano has 2 staves: treble & bass).

#### Understanding the Hierarchy:
- **Part**: A musical instrument (e.g., "Piano", "Violin")
- **Staff**: A single staff of music (e.g., piano has 2 staves: right hand = staff 0, left hand = staff 1)
- **Voice**: Each staff has 4 voices (0-3) for polyphonic music
- **Track**: Unique identifier = staffIdx * 4 + voice (e.g., staff 0 voice 1 = track 1)

#### Example: String Quartet Score Structure
```
Part 0: Violin I    -> Staff 0 (voices 0-3 = tracks 0-3)
Part 1: Violin II   -> Staff 1 (voices 0-3 = tracks 4-7)
Part 2: Viola       -> Staff 2 (voices 0-3 = tracks 8-11)
Part 3: Cello       -> Staff 3 (voices 0-3 = tracks 12-15)
```

#### Example: Piano Score Structure
```
Part 0: Piano -> Staff 0 (treble clef, right hand, tracks 0-3)
              -> Staff 1 (bass clef, left hand, tracks 4-7)
```

#### Getting Score Information

```notateai
var numStaves = curScore.nstaves;
var numMeasures = curScore.nmeasures;
var numTracks = curScore.ntracks;

api.log.info("Score has " + numStaves + " staves");
api.log.info("Score has " + numMeasures + " measures");

// List all instruments/parts
var parts = curScore.parts;
for (var i = 0; i < parts.length; i++) {
    var part = parts[i];
    api.log.info("Part " + i + ": " + part.partName);
    api.log.info("  Instrument ID: " + part.instrumentId);
    api.log.info("  Start track: " + part.startTrack);
    api.log.info("  End track: " + part.endTrack);
}
```

#### Working with a Specific Instrument/Staff

To modify notes for a specific instrument, set cursor.staffIdx:

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);

curScore.startCmd("Add Notes to Violin");

// Add to first staff (staffIdx 0) - e.g., Violin I
cursor.staffIdx = 0;
cursor.voice = 0;
cursor.setDuration(1, 4);
cursor.addNote(76);  // E5
cursor.addNote(74);  // D5

curScore.endCmd();
```

#### Working with Piano (Two Staves)

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);

curScore.startCmd("Add Piano Notes");

cursor.setDuration(1, 4);

// Right hand (treble clef) - usually staff 0
cursor.staffIdx = 0;
cursor.voice = 0;
cursor.addNote(72);  // C5
cursor.addNote(76);  // E5

// Left hand (bass clef) - usually staff 1
cursor.staffIdx = 1;
cursor.voice = 0;
cursor.rewind(Cursor.SCORE_START);  // Reset to start for this staff
cursor.addNote(48);  // C3
cursor.addNote(52);  // E3

curScore.endCmd();
```

#### Iterating Through ALL Staves

To apply changes to all instruments in the score:

```notateai
var cursor = curScore.newCursor();

curScore.startCmd("Add Notes to All Staves");

cursor.setDuration(1, 4);

// Loop through each staff
for (var staff = 0; staff < curScore.nstaves; staff++) {
    cursor.staffIdx = staff;
    cursor.voice = 0;
    cursor.rewind(Cursor.SCORE_START);

    // Add a C note in the middle register of each staff
    cursor.addNote(60);  // Middle C

    api.log.info("Added note to staff " + staff);
}

curScore.endCmd();
```

#### Finding Which Staff Belongs to Which Instrument

```notateai
// Example: User says "add notes to the violin"
var parts = curScore.parts;
var violinStaffIdx = -1;

for (var i = 0; i < parts.length; i++) {
    var partName = parts[i].partName.toLowerCase();
    api.log.info("Part " + i + ": " + parts[i].partName);

    if (partName.indexOf("violin") >= 0) {
        // Found the violin part!
        // Get the first staff of this part
        violinStaffIdx = parts[i].startTrack / 4;
        api.log.info("Found violin at staff index: " + violinStaffIdx);
        break;
    }
}

if (violinStaffIdx >= 0) {
    var cursor = curScore.newCursor();
    cursor.rewind(Cursor.SCORE_START);
    cursor.staffIdx = violinStaffIdx;
    cursor.voice = 0;

    curScore.startCmd("Add to Violin");
    cursor.setDuration(1, 4);
    cursor.addNote(76);  // E5
    curScore.endCmd();
} else {
    api.log.error("No violin part found in score");
}
```

#### Understanding Voices (Polyphony within a Staff)

Each staff has 4 voices (0-3) to allow multiple independent musical lines:
- Voice 0: Primary voice (stems usually up)
- Voice 1: Secondary voice (stems usually down)
- Voice 2-3: Additional voices

```notateai
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);
cursor.staffIdx = 0;

curScore.startCmd("Add Two Voice Parts");

cursor.setDuration(1, 4);

// Voice 0 (melody)
cursor.voice = 0;
cursor.addNote(72);  // C5
cursor.addNote(74);  // D5

// Voice 1 (harmony) - at the SAME time position
cursor.voice = 1;
cursor.rewind(Cursor.SCORE_START);  // Go back to start
cursor.addNote(60);  // C4
cursor.addNote(62);  // D4

curScore.endCmd();
```

IMPORTANT: When working with multiple staves/voices, remember to:
1. Set cursor.staffIdx before working with a different instrument
2. Call cursor.rewind() after changing staffIdx to position correctly
3. Check curScore.nstaves to avoid accessing non-existent staves
4. Use curScore.parts to identify instruments by name
)"
;

void GeminiService::sendMessage(const QString& userMessage, bool includeScoreData)
{
    LOGI() << "GeminiService::sendMessage called with message: " << userMessage;
    LOGI() << "Include score data: " << includeScoreData;

    // Store user message to add to history after successful response
    m_pendingUserMessage = userMessage;

    if (includeScoreData) {
        // STEP 3 VERIFICATION: Test score data extraction
        QString scoreData = extractScoreDataAsMusicXML();
        LOGI() << "=== SCORE DATA EXTRACTION TEST ===";
        LOGI() << "Score data length:" << scoreData.length();
        LOGI() << "First 500 chars:" << scoreData.left(500);
        LOGI() << "===================================";
    }

    auto callback = [this](GeminiResponse response) {
        LOGI() << "Callback lambda invoked! Success: " << response.success;
        LOGI() << "Callback: About to emit responseReceived signal";

        // Add to conversation history if successful
        if (response.success && !m_pendingUserMessage.isEmpty()) {
            // Add user message
            ConversationTurn userTurn;
            userTurn.role = "user";
            userTurn.text = m_pendingUserMessage;
            m_conversationHistory.append(userTurn);

            // Add AI response
            ConversationTurn modelTurn;
            modelTurn.role = "model";
            modelTurn.text = response.responseText;
            m_conversationHistory.append(modelTurn);

            LOGI() << "Conversation history updated, now has" << m_conversationHistory.size() << "turns";
            m_pendingUserMessage.clear();
        }

        // Emit the signal - Qt will handle cross-thread communication automatically
        emit responseReceived(response);

        LOGI() << "Callback: responseReceived signal emitted";
    };

    LOGI() << "Starting concurrent task...";
    // Use lambda to capture all parameters since Concurrent::run only supports up to 2 params
    Concurrent::run([this, userMessage, includeScoreData, callback]() {
        th_sendMessageDirect(userMessage, includeScoreData, callback);
    });
    LOGI() << "Concurrent task started";
}

void GeminiService::clearHistory()
{
    m_conversationHistory.clear();
    m_pendingUserMessage.clear();
    LOGI() << "Conversation history cleared";
}

void GeminiService::th_sendMessage(const QString& userMessage, bool includeScoreData, std::function<void(GeminiResponse)> callback)
{
    TRACEFUNC;

    GeminiResponse response;

    // Check if API key is configured
    QString apiKey = configuration()->geminiApiKey();
    if (apiKey.isEmpty()) {
        LOGE() << "Gemini API key is not configured";
        response.success = false;
        response.errorMessage = "API key not configured. Please add your Gemini API key in Preferences.";
        callback(response);
        return;
    }

    // Build the request URL with API key
    QUrl requestUrl(GEMINI_API_ENDPOINT);
    QUrlQuery query;
    query.addQueryItem("key", apiKey);
    requestUrl.setQuery(query);

    // Create network manager
    INetworkManagerPtr networkManager = networkManagerCreator()->makeNetworkManager();

    // Build request JSON
    QByteArray requestJson = buildRequestJson(userMessage, includeScoreData);

    // Setup request headers
    RequestHeaders headers;
    headers.knownHeaders[QNetworkRequest::ContentTypeHeader] = "application/json";

    // Prepare buffers for request and response
    QBuffer requestBuffer;
    requestBuffer.setData(requestJson);
    requestBuffer.open(QIODevice::ReadOnly);

    QBuffer responseBuffer;

    OutgoingDevice outgoingDevice(&requestBuffer);

    LOGI() << "Sending request to Gemini API...";
    LOGD() << "Request URL: " << requestUrl.toString();
    LOGD() << "Request JSON: " << requestJson;

    // Send POST request
    Ret result = networkManager->post(requestUrl, &outgoingDevice, &responseBuffer, headers);

    if (!result) {
        LOGE() << "Network request failed: " << result.toString();
        LOGE() << "Error code: " << result.code();
        LOGE() << "Error text: " << result.text();

        // Check if there's any response data even on error
        QByteArray errorData = responseBuffer.data();
        if (!errorData.isEmpty()) {
            LOGE() << "Response data on error: " << errorData;
        }

        response.success = false;

        // Provide user-friendly error messages
        if (result.code() == static_cast<int>(Ret::Code::UnknownError)) {
            response.errorMessage = "Network error. Please check your internet connection.";
        } else {
            response.errorMessage = QString("Request failed: %1").arg(QString::fromStdString(result.text()));
        }

        callback(response);
        return;
    }

    // Parse response
    QByteArray responseData = responseBuffer.data();
    LOGD() << "Response received: " << responseData;

    QJsonParseError parseError;
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        LOGE() << "Failed to parse JSON response: " << parseError.errorString();
        response.success = false;
        response.errorMessage = "Failed to parse API response.";
        callback(response);
        return;
    }

    // Parse the response
    response = parseResponse(responseDoc);

    if (response.success) {
        LOGI() << "Successfully received AI response";
    } else {
        LOGW() << "API returned error: " << response.errorMessage;
    }

    callback(response);
}

QString GeminiService::extractScoreDataAsMusicXML() const
{
    // Get current notation from global context (call Inject as a function)
    notation::INotationPtr notation = m_globalContext()->currentNotation();

    if (!notation) {
        return QString("No score currently open");
    }

    // Get the underlying Score object
    mu::engraving::Score* score = notation->elements()->msScore();

    if (!score) {
        return QString("Score data unavailable");
    }

    // Export to MusicXML format using muse::io::Buffer
    muse::io::Buffer buffer;
    buffer.open(muse::io::IODevice::OpenMode::WriteOnly);

    // Use the MusicXML writer to export
    bool success = mu::iex::musicxml::saveXml(score, &buffer);

    if (!success) {
        return QString("Error exporting score to MusicXML");
    }

    // Convert buffer to string
    muse::ByteArray data = buffer.data();
    QString musicXml = QString::fromUtf8(data.constChar(), data.size());

    return musicXml;
}

QByteArray GeminiService::buildRequestJson(const QString& userMessage, bool includeScoreData) const
{
    QJsonObject requestObj;

    // Build system prompt - include API docs only on first message (empty conversation history)
    bool isFirstMessage = m_conversationHistory.isEmpty();
    QString systemPrompt = BASE_SYSTEM_PROMPT;

    if (isFirstMessage) {
        // Include full API documentation on first message
        systemPrompt += API_DOCUMENTATION;
        LOGI() << "=== FIRST MESSAGE - Including API documentation ===";
    } else {
        LOGI() << "=== SUBSEQUENT MESSAGE - API docs already in context ===";
    }

    if (includeScoreData) {
        // Extract current score data only if requested
        QString scoreData = extractScoreDataAsMusicXML();

        if (!scoreData.isEmpty() &&
            scoreData != "No score currently open" &&
            scoreData != "Score data unavailable" &&
            !scoreData.startsWith("Error exporting score")) {
            systemPrompt += "\n\n## Current Score Context\n\n";
            systemPrompt += "The user is currently working on the following musical score (in MusicXML format):\n\n";
            systemPrompt += "```xml\n";
            systemPrompt += scoreData;
            systemPrompt += "\n```\n\n";
            systemPrompt += "Please use this score data to provide contextually relevant responses about the user's music.";

            LOGI() << "=== SCORE DATA INCLUDED ===";
            LOGI() << "Score data length:" << scoreData.length() << "characters";
            LOGI() << "===========================";
        }
    } else {
        LOGI() << "Score data NOT included in this request";
    }

    // System instruction
    QJsonObject systemInstructionObj;
    QJsonArray systemPartsArray;
    QJsonObject systemPartObj;
    systemPartObj["text"] = systemPrompt;
    systemPartsArray.append(systemPartObj);
    systemInstructionObj["parts"] = systemPartsArray;
    requestObj["systemInstruction"] = systemInstructionObj;

    // Build the contents array with conversation history
    QJsonArray contentsArray;

    // Add previous conversation turns
    for (const ConversationTurn& turn : m_conversationHistory) {
        QJsonObject turnObj;
        turnObj["role"] = turn.role;

        QJsonArray partsArray;
        QJsonObject partObj;
        partObj["text"] = turn.text;
        partsArray.append(partObj);

        turnObj["parts"] = partsArray;
        contentsArray.append(turnObj);
    }

    // Add current user message
    QJsonObject currentMessageObj;
    currentMessageObj["role"] = "user";

    QJsonArray currentPartsArray;
    QJsonObject currentPartObj;
    currentPartObj["text"] = userMessage;
    currentPartsArray.append(currentPartObj);

    currentMessageObj["parts"] = currentPartsArray;
    contentsArray.append(currentMessageObj);

    requestObj["contents"] = contentsArray;

    QJsonDocument doc(requestObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    // Log request details
    LOGI() << "=== REQUEST JSON DETAILS ===";
    LOGI() << "First message (includes API docs):" << isFirstMessage;
    LOGI() << "Total request size:" << jsonData.size() << "bytes";
    LOGI() << "Conversation history turns:" << m_conversationHistory.size();
    LOGI() << "Include score data:" << includeScoreData;
    LOGI() << "=============================";

    return jsonData;
}

GeminiService::GeminiResponse GeminiService::parseResponse(const QJsonDocument& responseDoc) const
{
    GeminiResponse response;

    QJsonObject rootObj = responseDoc.object();

    // Check for error in response
    if (rootObj.contains("error")) {
        QJsonObject errorObj = rootObj["error"].toObject();
        QString errorMessage = errorObj["message"].toString();
        int errorCode = errorObj["code"].toInt();

        LOGW() << "API error code: " << errorCode << ", message: " << errorMessage;

        response.success = false;

        // Provide user-friendly error messages
        if (errorCode == 401 || errorCode == 403) {
            response.errorMessage = "API authentication failed. Please check your API key in Preferences.";
        } else if (errorCode == 429) {
            response.errorMessage = "API rate limit exceeded. Please wait and try again.";
        } else {
            response.errorMessage = QString("API error: %1").arg(errorMessage);
        }

        return response;
    }

    // Parse successful response
    if (!rootObj.contains("candidates")) {
        LOGW() << "Response missing 'candidates' field";
        response.success = false;
        response.errorMessage = "Invalid API response format.";
        return response;
    }

    QJsonArray candidates = rootObj["candidates"].toArray();
    if (candidates.isEmpty()) {
        LOGW() << "No candidates in response";
        response.success = false;
        response.errorMessage = "No response generated.";
        return response;
    }

    QJsonObject firstCandidate = candidates[0].toObject();
    QJsonObject content = firstCandidate["content"].toObject();
    QJsonArray parts = content["parts"].toArray();

    if (parts.isEmpty()) {
        LOGW() << "No parts in candidate response";
        response.success = false;
        response.errorMessage = "Empty response from API.";
        return response;
    }

    QJsonObject firstPart = parts[0].toObject();
    QString text = firstPart["text"].toString();

    if (text.isEmpty()) {
        LOGW() << "Empty text in response";
        response.success = false;
        response.errorMessage = "Empty response from API.";
        return response;
    }

    response.success = true;
    response.responseText = text;

    return response;
}

void GeminiService::th_sendMessageDirect(const QString& userMessage, bool includeScoreData, std::function<void(GeminiResponse)> callback)
{
    TRACEFUNC;

    GeminiResponse response;

    // Check if API key is configured
    QString apiKey = configuration()->geminiApiKey();
    if (apiKey.isEmpty()) {
        LOGE() << "Gemini API key is not configured";
        response.success = false;
        response.errorMessage = "API key not configured. Please add your Gemini API key in Preferences.";
        callback(response);
        return;
    }

    // Build the request URL with API key
    QUrl requestUrl(GEMINI_API_ENDPOINT);
    QUrlQuery query;
    query.addQueryItem("key", apiKey);
    requestUrl.setQuery(query);

    // Build request JSON
    QByteArray requestJson = buildRequestJson(userMessage, includeScoreData);

    LOGI() << "Sending request to Gemini API using direct QNetworkAccessManager...";
    LOGD() << "Request URL: " << requestUrl.toString();
    LOGD() << "Request JSON: " << requestJson;

    // Create network request
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    // Create network manager
    QNetworkAccessManager* manager = new QNetworkAccessManager();

    // Handle SSL errors - ignore them for now (for debugging)
    QObject::connect(manager, &QNetworkAccessManager::sslErrors,
                     [](QNetworkReply* reply, const QList<QSslError>& errors) {
        LOGW() << "SSL errors occurred:";
        for (const QSslError& error : errors) {
            LOGW() << "  - " << error.errorString();
        }
        // Ignore SSL errors for Google's API
        reply->ignoreSslErrors();
    });

    // Send POST request
    QNetworkReply* reply = manager->post(request, requestJson);

    // Also connect SSL errors directly to the reply
    QObject::connect(reply, QOverload<const QList<QSslError>&>::of(&QNetworkReply::sslErrors),
                     [](const QList<QSslError>& errors) {
        LOGW() << "SSL errors on reply:";
        for (const QSslError& error : errors) {
            LOGW() << "  - " << error.errorString();
        }
    });

    // Wait for reply
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool isTimeout = false;
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&isTimeout, reply]() {
        isTimeout = true;
        reply->abort();
    });

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timeoutTimer.start(60000); // 60 second timeout
    loop.exec();

    if (isTimeout) {
        LOGE() << "Request timed out";
        response.success = false;
        response.errorMessage = "Request timed out. Please try again.";
        reply->deleteLater();
        manager->deleteLater();
        callback(response);
        return;
    }

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        LOGE() << "Network error: " << reply->errorString();
        LOGE() << "Error code: " << reply->error();

        // Log HTTP status code
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid()) {
            LOGE() << "HTTP status code: " << statusCode.toInt();
        }

        // Log response data even on error
        QByteArray errorResponseData = reply->readAll();
        if (!errorResponseData.isEmpty()) {
            LOGE() << "Error response data: " << errorResponseData;
        }

        response.success = false;

        // Provide user-friendly error messages
        switch (reply->error()) {
        case QNetworkReply::AuthenticationRequiredError:
            response.errorMessage = "API authentication failed. Please check your API key in Preferences.";
            break;
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::NetworkSessionFailedError:
            response.errorMessage = "Network error. Please check your internet connection.";
            break;
        default:
            response.errorMessage = QString("Request failed: %1").arg(reply->errorString());
            break;
        }

        reply->deleteLater();
        manager->deleteLater();
        callback(response);
        return;
    }

    // Read response
    QByteArray responseData = reply->readAll();
    LOGD() << "Response received: " << responseData;

    reply->deleteLater();
    manager->deleteLater();

    // Parse response
    QJsonParseError parseError;
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        LOGE() << "Failed to parse JSON response: " << parseError.errorString();
        response.success = false;
        response.errorMessage = "Failed to parse API response.";
        callback(response);
        return;
    }

    // Parse the response
    response = parseResponse(responseDoc);

    if (response.success) {
        LOGI() << "Successfully received AI response";
        LOGI() << "AI Response text: " << response.responseText;
        LOGD() << "Response text length: " << response.responseText.length() << " characters";
    } else {
        LOGW() << "API returned error: " << response.errorMessage;
    }

    LOGD() << "About to call callback with response (success=" << response.success << ")";
    callback(response);
    LOGD() << "Callback invoked successfully";
}
