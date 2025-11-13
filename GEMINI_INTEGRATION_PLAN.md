# Implementation Plan: Google Gemini API Integration for NotateAI Panel

## Overview
Integrate Google Gemini API into the existing NotateAI chat panel in MuseScore Studio (Qt/QML application). The panel currently has a working UI with placeholder responses that need to be replaced with real AI responses from Gemini.

---

## Step 1: Create NotateAI Module Structure
**What to implement:**
- Create `src/notateai/CMakeLists.txt` with module configuration
- Create `src/notateai/notateaimodule.h/cpp` for module registration
- Create `src/notateai/notateaitypes.h` for shared types/enums
- Update `src/CMakeLists.txt` to include the notateai subdirectory
- Register module in the application's module system

**Visual verification:**
- Project builds successfully without errors
- Check CMake output shows "NotateAI module" being configured
- Run MuseScore and verify the NotateAI panel still appears (no regressions)

---

## Step 2: Create Configuration System for API Key
**What to implement:**
- Create `src/notateai/inotateaiconfiguration.h` interface with methods:
  - `QString geminiApiKey()`
  - `void setGeminiApiKey(QString)`
- Create `src/notateai/internal/notateaiconfiguration.h/cpp` implementation
  - Use MuseScore's settings framework to persist the API key
- Update module to register configuration in IoC container

**Visual verification:**
- Build succeeds
- Use Qt Creator debugger or add temporary logging to verify configuration class instantiates
- API key defaults to empty string (check via debugger)

---

## Step 3: Create Gemini Service for API Communication
**What to implement:**
- Create `src/notateai/internal/geminiservice.h/cpp` with:
  - Method: `async::Channel<QString> sendMessage(QString userMessage)`
  - Inject `INetworkManagerCreator` dependency
  - Implement POST request to Gemini API endpoint: `https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent`
  - Parse JSON response and extract AI text
  - Handle errors (network failures, API errors, rate limits)

**Visual verification:**
- Build succeeds
- Add temporary hardcoded API key in the service
- Add logging/qDebug statements before and after API call
- Test by triggering a message send and checking logs for:
  - Request being sent
  - Response received (or error message)
  - Parsed AI text output

---

## Step 4: Create View Model to Connect QML and Backend
**What to implement:**
- Create `src/notateai/view/notateaipanelmodel.h/cpp` (inheriting QObject)
- Properties:
  - `Q_INVOKABLE void sendMessage(QString message)`
  - Signal: `messageReceived(QString aiResponse)`
  - Signal: `errorOccurred(QString errorMessage)`
  - Property: `bool isLoading` (for showing loading state)
- Inject GeminiService dependency
- Wire up async response handling

**Visual verification:**
- Build succeeds
- Add qDebug logging in sendMessage() method
- Temporarily call `sendMessage("test")` from QML onCompleted handler
- Check console logs confirm method is being called

---

## Step 5: Update NotateAIPanel.qml to Use View Model
**What to implement:**
- Add C++ model registration in module
- Replace local `addMessage()` function with calls to view model
- Add `NotateAIPanelModel` instance to QML
- Connect send button to `model.sendMessage(textArea.text)`
- Add signal handlers:
  - `onMessageReceived`: Add AI message to list
  - `onErrorOccurred`: Show error in UI
- Add loading indicator (busy indicator) while `model.isLoading === true`

**Visual verification:**
- Build and run MuseScore
- Open NotateAI panel (View menu → NotateAI)
- Type "Hello" and click send
- **Expected behavior:**
  - User message appears immediately
  - Loading indicator shows briefly
  - AI response appears (if API key is valid) OR error message shows
- Check console logs for request/response flow

---

## Step 6: Add API Key Configuration UI
**What to implement:**
- Create `src/notateai/qml/NotateAIPreferencesPage.qml` with:
  - TextField for API key input (password echo mode)
  - Link to Google AI Studio for getting API key
  - Save button
- Register preferences page in MuseScore's preferences system
- Connect to configuration backend

**Visual verification:**
- Build and run MuseScore
- Open Edit → Preferences → NotateAI (new section)
- Enter API key: `test-key-12345`
- Click Save
- Restart MuseScore
- Open Preferences again → verify API key persisted (shows as asterisks)
- Check settings file (typically in `%APPDATA%/MuseScore/MuseScore4.ini` on Windows) contains encrypted/encoded key

---

## Step 7: Improve UX with Message History & Error Handling
**What to implement:**
- Add message history persistence (save last N messages)
- Add retry mechanism for failed requests
- Add proper error messages:
  - "No API key configured - please add one in Preferences"
  - "Network error - check your connection"
  - "API rate limit exceeded - please wait"
- Add message timestamps (optional)
- Add "Clear conversation" button

**Visual verification:**
- Build and run MuseScore
- **Test 1 - No API key:**
  - Remove API key from preferences
  - Send message → See friendly error: "Please configure API key in Preferences"
- **Test 2 - Invalid API key:**
  - Set API key to "invalid-key"
  - Send message → See error: "API authentication failed"
- **Test 3 - Network offline:**
  - Disconnect internet
  - Send message → See error: "Network error - check connection"
- **Test 4 - Message persistence:**
  - Have a conversation with 3-4 messages
  - Close and reopen MuseScore
  - Open NotateAI panel → previous messages should still be visible

---

## Step 8: Polish and Optimization
**What to implement:**
- Add request cancellation (if user closes panel while request pending)
- Implement request timeout (e.g., 30 seconds)
- Add typing indicator animation
- Improve message bubble styling (markdown support for code blocks, etc.)
- Add rate limiting on client side (prevent spam clicking)
- Add unit tests for GeminiService

**Visual verification:**
- Build and run MuseScore
- **Test 1 - Cancellation:**
  - Send message
  - Immediately close NotateAI panel while loading
  - Reopen panel → no orphaned loading state
- **Test 2 - Timeout:**
  - (Simulate slow network with dev tools)
  - Send message → after 30s, see timeout error
- **Test 3 - Typing indicator:**
  - Send message → see animated "..." or typing indicator while waiting
- **Test 4 - Spam prevention:**
  - Click send button rapidly 5 times
  - Only 1 request should be in flight
  - Button should be disabled while loading

---

## Notes:
- Each step builds on the previous one
- Testing at each step ensures we catch issues early
- API key should NEVER be committed to git
- Follow MuseScore's existing code style and patterns throughout

---

## Architecture Overview

### Current Structure
```
MuseScore Studio (Qt/QML Application)
├── src/appshell/
│   └── qml/NotationPage/
│       └── NotateAIPanel.qml (existing UI with placeholder responses)
└── src/notateai/ (empty - to be populated)
```

### Target Structure
```
src/notateai/
├── CMakeLists.txt
├── notateaimodule.h/cpp
├── notateaitypes.h
├── inotateaiconfiguration.h
├── internal/
│   ├── notateaiconfiguration.h/cpp
│   └── geminiservice.h/cpp
├── view/
│   └── notateaipanelmodel.h/cpp
└── qml/
    └── NotateAIPreferencesPage.qml
```

### Key Design Patterns
- **Dependency Injection**: Using MuseScore's IoC container
- **Async Communication**: Using `async::Channel<T>` for non-blocking API calls
- **MVVM Pattern**: QML views bound to C++ view models
- **Configuration**: Settings persisted via MuseScore's settings framework

---

## API Integration Details

### Gemini API Endpoint
```
POST https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=YOUR_API_KEY
```

### Request Format
```json
{
  "contents": [{
    "parts": [{
      "text": "User message here"
    }]
  }]
}
```

### Response Format
```json
{
  "candidates": [{
    "content": {
      "parts": [{
        "text": "AI response here"
      }]
    }
  }]
}
```

### Getting an API Key
1. Visit [Google AI Studio](https://makersuite.google.com/app/apikey)
2. Click "Create API Key"
3. Copy the key and enter it in MuseScore Preferences → NotateAI

---

## Security Considerations
- Never hardcode API keys in source code
- Store API keys in user preferences (encrypted if possible via Qt's secure storage)
- Validate all API responses before displaying
- Handle rate limiting gracefully
- Add `.env` or config files to `.gitignore` if used for development

---

## Testing Strategy
- **Unit Tests**: Test GeminiService JSON parsing and error handling
- **Integration Tests**: Test full flow from QML → Service → API
- **Manual Testing**: Follow visual verification steps at each stage
- **Error Scenarios**: Test all error paths (no API key, network errors, invalid responses)

---

## Future Enhancements (Beyond Initial Implementation)
- Support for conversation context (multi-turn conversations)
- Support for different Gemini models (gemini-pro, gemini-pro-vision)
- Image input support (attach images to chat)
- Export conversation history
- Customizable system prompts
- Token usage tracking and cost estimation
