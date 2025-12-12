# NotateAI - MuseScore 4 API Documentation

This documentation describes how to programmatically modify MuseScore scores using the internal API. This is essential for the NotateAI integration where AI commands need to be translated into score modifications.

## Table of Contents

1. [Overview](#overview)
2. [API Architecture](#api-architecture)
3. [Quick Reference](#quick-reference)
4. [Detailed Documentation](#detailed-documentation)

## Overview

MuseScore 4 provides two main approaches for programmatic score modification:

1. **Plugin API (QML/JavaScript)** - High-level API exposed to plugins via `MuseScore` QML type
2. **Actions/Commands System** - Internal action dispatch system for UI commands

For NotateAI integration, we can use either:
- The Plugin API's `cmd()` function to dispatch action codes
- Direct cursor-based manipulation for note input
- The actions dispatcher for complex commands

## API Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    NotateAI Panel (QML)                     │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│              Plugin API (MuseScore QML Type)                │
│  - cmd(action)        Execute action by name                │
│  - curScore           Access current score                  │
│  - newElement()       Create elements                       │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                    Score API                                │
│  - newCursor()        Create cursor for navigation          │
│  - startCmd()/endCmd() Undo/redo transaction                │
│  - appendMeasures()   Add measures                          │
│  - selection          Access selection                      │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                    Cursor API                               │
│  - addNote(pitch)     Add note at cursor                    │
│  - addRest()          Add rest                              │
│  - setDuration(z,n)   Set duration                          │
│  - rewind(mode)       Navigate                              │
└─────────────────────────────────────────────────────────────┘
```

## Quick Reference

### Adding Notes

```javascript
// Get cursor for current score
var cursor = curScore.newCursor();

// Navigate to start
cursor.rewind(Cursor.SCORE_START);

// Set duration (1/4 = quarter note)
cursor.setDuration(1, 4);

// Add a note (MIDI pitch 60 = Middle C)
curScore.startCmd("Add Note");
cursor.addNote(60);
curScore.endCmd();
```

### Common Actions via cmd()

```javascript
// Navigation
cmd("notation-move-right");     // Next chord
cmd("notation-move-left");      // Previous chord
cmd("first-element");           // Go to start
cmd("last-element");            // Go to end

// Note input
cmd("note-input");              // Toggle note input mode
cmd("note-c");                  // Enter C
cmd("note-d");                  // Enter D
cmd("rest");                    // Enter rest

// Editing
cmd("action://notation/delete"); // Delete selection
cmd("action://notation/copy");   // Copy
cmd("action://notation/paste");  // Paste
cmd("pitch-up");                 // Pitch up
cmd("pitch-down");               // Pitch down
```

## Detailed Documentation

See the following files for detailed information:

- [Cursor API](./cursor-api.md) - Navigation and note input
- [Score API](./score-api.md) - Score manipulation
- [Actions Reference](./actions-reference.md) - Complete list of action codes
- [Elements Reference](./elements-reference.md) - Element types and properties
- [Pitch Reference](./pitch-reference.md) - MIDI pitch and TPC values
- [Duration Reference](./duration-reference.md) - Tick lengths and duration values

## Source Files Reference

Key source files for understanding the API:

| File | Description |
|------|-------------|
| `src/engraving/api/v1/qmlpluginapi.h` | Main Plugin API interface |
| `src/engraving/api/v1/cursor.h` | Cursor class for note input |
| `src/engraving/api/v1/score.h` | Score class with modification methods |
| `src/engraving/api/v1/elements.h` | All element types |
| `src/notation/internal/notationuiactions.cpp` | All UI action definitions |
| `src/notation/inotationinteraction.h` | Interaction interface |
| `src/notation/inotationnoteinput.h` | Note input interface |
