# Feature Implementation: Send Score Data to Gemini AI

## Overview
Automatically include the currently open music score's data (as MusicXML) with every user prompt sent to Gemini, providing complete musical context for AI responses.

---

## Implementation Steps

### Step 1: Add Global Context Dependency Injection
**Files to modify:**
- `src/notateai/internal/geminiservice.h`

**Changes:**
```cpp
// Add to class member variables (around line 30-35)
muse::Inject<mu::context::IGlobalContext> m_globalContext = { this };
```

**Verification:**
- Compile the project - should build without errors
- No runtime verification needed yet

---

### Step 2: Add Required Headers and Dependencies
**Files to modify:**
- `src/notateai/internal/geminiservice.cpp`

**Changes:**
```cpp
// Add these includes at the top of the file
#include "context/iglobalcontext.h"
#include "notation/inotation.h"
#include "notation/inotationelements.h"
#include "engraving/dom/score.h"
#include "engraving/dom/masterscore.h"
#include "importexport/musicxml/internal/musicxml/exportxml.h"
#include <QBuffer>
```

**Verification:**
- Compile the project - should build without errors
- Check that headers are found correctly

---

### Step 3: Implement Score Data Extraction Method (MusicXML Export)
**Files to modify:**
- `src/notateai/internal/geminiservice.h`
- `src/notateai/internal/geminiservice.cpp`

**Add to geminiservice.h (around line 28-30, before signals):**
```cpp
private:
    QString extractScoreDataAsMusicXML() const;
```

**Add to geminiservice.cpp (after buildRequestJson method):**
```cpp
QString GeminiService::extractScoreDataAsMusicXML() const
{
    // Get current notation from global context
    notation::INotationPtr notation = m_globalContext->currentNotation();

    if (!notation) {
        return QString("No score currently open");
    }

    // Get the underlying Score object
    mu::engraving::Score* score = notation->elements()->msScore();

    if (!score) {
        return QString("Score data unavailable");
    }

    // Export to MusicXML format
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    // Use the MusicXML writer to export
    muse::Ret ret = mu::engraving::ExportMusicXml::write(score, buffer, false);

    if (!ret) {
        return QString("Error exporting score: ") + QString::fromStdString(ret.toString());
    }

    // Convert buffer to string
    QString musicXml = QString::fromUtf8(buffer.data());

    return musicXml;
}
```

**Verification Method:**
1. Add temporary logging in `sendMessage()` method:
   ```cpp
   void GeminiService::sendMessage(const QString& userMessage)
   {
       QString scoreData = extractScoreDataAsMusicXML();
       qDebug() << "=== SCORE DATA EXTRACTION TEST ===";
       qDebug() << "Score data length:" << scoreData.length();
       qDebug() << "First 500 chars:" << scoreData.left(500);
       qDebug() << "===================================";

       // ... rest of existing code
   }
   ```

2. Build and run NotateAI
3. Open a score file (any .mscz file)
4. Open NotateAI panel
5. Type any message and send
6. Check debug console output - should see:
   - Score data length > 0
   - MusicXML header like `<?xml version="1.0" encoding="UTF-8"?>`
   - If no score open, should see "No score currently open"

---

### Step 4: Modify buildRequestJson to Include Score Context
**Files to modify:**
- `src/notateai/internal/geminiservice.cpp`

**Modify the `buildRequestJson` method (around line 69-91):**
```cpp
QByteArray GeminiService::buildRequestJson(const QString& userMessage) const
{
    QJsonObject requestObj;

    // Extract current score data
    QString scoreData = extractScoreDataAsMusicXML();

    // Enhanced system prompt with score context
    QString enhancedSystemPrompt = SYSTEM_PROMPT;

    if (!scoreData.isEmpty() &&
        scoreData != "No score currently open" &&
        scoreData != "Score data unavailable") {
        enhancedSystemPrompt += "\n\n## Current Score Context\n\n";
        enhancedSystemPrompt += "The user is currently working on the following musical score (in MusicXML format):\n\n";
        enhancedSystemPrompt += "```xml\n";
        enhancedSystemPrompt += scoreData;
        enhancedSystemPrompt += "\n```\n\n";
        enhancedSystemPrompt += "Please use this score data to provide contextually relevant responses about the user's music.";
    }

    // System instruction with enhanced prompt
    QJsonObject systemInstructionObj;
    QJsonArray systemPartsArray;
    QJsonObject systemPartObj;
    systemPartObj["text"] = enhancedSystemPrompt;
    systemPartsArray.append(systemPartObj);
    systemInstructionObj["parts"] = systemPartsArray;
    requestObj["systemInstruction"] = systemInstructionObj;

    // User message contents (unchanged)
    QJsonArray contentsArray;
    QJsonObject contentObj;
    QJsonArray partsArray;
    QJsonObject partObj;
    partObj["text"] = userMessage;
    partsArray.append(partObj);
    contentObj["parts"] = partsArray;
    contentsArray.append(contentObj);
    requestObj["contents"] = contentsArray;

    return QJsonDocument(requestObj).toJson(QJsonDocument::Compact);
}
```

**Verification Method:**
1. Add temporary logging in `buildRequestJson()`:
   ```cpp
   QByteArray GeminiService::buildRequestJson(const QString& userMessage) const
   {
       // ... all the code above ...

       QByteArray jsonData = QJsonDocument(requestObj).toJson(QJsonDocument::Compact);

       qDebug() << "=== REQUEST JSON TEST ===";
       qDebug() << "Total request size:" << jsonData.size() << "bytes";
       qDebug() << "Contains MusicXML:" << jsonData.contains("MusicXML");
       qDebug() << "Contains score-partwise:" << jsonData.contains("score-partwise");
       qDebug() << "=======================";

       return jsonData;
   }
   ```

2. Build and run NotateAI
3. Open a score file
4. Send a message through NotateAI panel
5. Check debug output - should see:
   - Request size significantly larger than before (likely 50KB+ depending on score)
   - `Contains MusicXML: true`
   - `Contains score-partwise: true`

---

### Step 5: Add Visual Indicator in UI Panel
**Files to modify:**
- `src/notateai/view/notateaipanelmodel.h`
- `src/notateai/view/notateaipanelmodel.cpp`
- `src/appshell/qml/NotationPage/NotateAIPanel.qml`

**Add to notateaipanelmodel.h (around line 25-30):**
```cpp
Q_PROPERTY(QString scoreContextInfo READ scoreContextInfo NOTIFY scoreContextInfoChanged)

signals:
    void scoreContextInfoChanged();

public:
    QString scoreContextInfo() const;

private slots:
    void updateScoreContextInfo();
```

**Add to notateaipanelmodel.cpp:**
```cpp
// In constructor or init method, connect to notation changes
void NotateAIPanelModel::init()
{
    // ... existing code ...

    updateScoreContextInfo();

    // Update context info when notation changes
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        updateScoreContextInfo();
    });
}

void NotateAIPanelModel::updateScoreContextInfo()
{
    notation::INotationPtr notation = globalContext()->currentNotation();

    if (!notation) {
        m_scoreContextInfo = "No score open - AI will have no musical context";
    } else {
        mu::engraving::Score* score = notation->elements()->msScore();
        if (score) {
            m_scoreContextInfo = QString("Score context: %1 - %2 measures")
                                .arg(score->title().isEmpty() ? "Untitled" : score->title())
                                .arg(score->nmeasures());
        } else {
            m_scoreContextInfo = "Score data unavailable";
        }
    }

    emit scoreContextInfoChanged();
}

QString NotateAIPanelModel::scoreContextInfo() const
{
    return m_scoreContextInfo;
}
```

**Add to NotateAIPanel.qml (around line 40-50, before the TextArea):**
```qml
// Score context indicator
Rectangle {
    width: parent.width
    height: 30
    color: "#E3F2FD"
    border.color: "#2196F3"
    border.width: 1
    radius: 4

    Text {
        anchors.centerIn: parent
        text: "📊 " + notateAIPanelModel.scoreContextInfo
        font.pixelSize: 12
        color: "#1976D2"
    }
}
```

**Verification Method:**
1. Build and run NotateAI
2. Open NotateAI panel WITHOUT opening a score
   - Should see: "No score open - AI will have no musical context"
3. Open a score file (e.g., any .mscz)
   - Should see: "Score context: [Title] - [X] measures"
   - Title and measure count should match the actual score
4. Close the score
   - Should update back to "No score open..."
5. Open a different score
   - Should update to show new score's information

---

### Step 6: End-to-End Functional Test
**Verification Method:**

1. **Prepare test score:**
   - Create or open a simple score with recognizable content
   - E.g., "Twinkle Twinkle Little Star" or a score with specific notes in C major

2. **Test AI understanding:**
   - Open the score in NotateAI
   - Send prompt: "What key is this piece in?"
   - AI should respond with the actual key from the score

3. **Test AI analysis:**
   - Send prompt: "What notes are in the first measure?"
   - AI should list the actual notes from the MusicXML data

4. **Test without score:**
   - Close all scores
   - Send prompt: "What key is this piece in?"
   - AI should respond that no score is open

5. **Test score switching:**
   - Open score in C major
   - Send prompt: "What key is this in?"
   - Note the response
   - Open different score in a different key (e.g., G major)
   - Send same prompt
   - AI should respond with the NEW key, proving it's reading current score

---

## Visual Verification Summary

Each step has a clear verification method:

| Step | Verification Type | Expected Result |
|------|------------------|-----------------|
| 1-2  | Compile-time | No build errors |
| 3    | Debug console | MusicXML output visible in logs |
| 4    | Debug console | Request contains MusicXML tags |
| 5    | UI Visual | Blue info bar shows score info |
| 6    | AI Response | AI answers questions about actual score content |

---

## Troubleshooting

**If MusicXML export fails:**
- Check that `ExportMusicXml::write()` function signature is correct
- May need to adjust include path or namespace
- Alternative: Use `mu::engraving::saveXml(score, &buffer)` function

**If context info doesn't update:**
- Verify `globalContext()->currentNotationChanged()` signal connection
- Check that `updateScoreContextInfo()` is being called

**If AI doesn't understand score data:**
- Verify MusicXML is actually in the request (Step 4 verification)
- Check Gemini API token limits (1M tokens for Flash)
- Very large scores might exceed limits - test with small scores first

---

## Token Usage Considerations

- Small score (1 page): ~10-20KB MusicXML = ~2,500-5,000 tokens
- Medium score (5 pages): ~50-100KB MusicXML = ~12,500-25,000 tokens
- Large score (20+ pages): ~200KB+ MusicXML = ~50,000+ tokens
- Gemini Flash limit: 1,000,000 tokens (plenty of headroom)

**Future optimization:** If needed, can add options later for:
- Metadata-only mode
- Summary mode
- Current selection only mode
