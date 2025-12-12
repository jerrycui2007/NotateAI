# Cursor API

The Cursor is the primary interface for navigating within a score and adding notes/rests. It represents a position within the score and provides methods for note input.

## Source File
`src/engraving/api/v1/cursor.h`

## Creating a Cursor

```javascript
var cursor = curScore.newCursor();
```

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `track` | int | Current track (staffIdx * 4 + voice). Read/write. |
| `staffIdx` | int | Current staff number (track / 4). Read/write. |
| `staff` | Staff | Current Staff object. Read/write. (Since 4.6) |
| `voice` | int | Current voice (track % 4, range 0-3). Read/write. |
| `filter` | int | Segment type filter bitmask. Determines which segments cursor moves to. |
| `tick` | int | MIDI tick position (read only). Deprecated in 4.6, use `fraction`. |
| `utick` | int | MIDI tick position accounting for repeats (read only). Since 4.6. |
| `fraction` | FractionWrapper | Time position as fraction of whole note. Since 4.6. |
| `tempo` | qreal | Tempo at current tick (read only). |
| `keySignature` | int | Key signature of current staff at tick (read only). |
| `score` | Score | Associated score. Read/write. |
| `element` | EngravingItem | Current element at track (read only). |
| `segment` | Segment | Current segment (read only). |
| `measure` | Measure | Current measure (read only). |
| `stringNumber` | int | Physical string number for tablature. Since 3.5. |
| `inputStateMode` | InputStateMode | Cursor input state behavior. Since 3.5. |

## Rewind Modes

```javascript
Cursor.SCORE_START      // 0 - Rewind to start of score
Cursor.SELECTION_START  // 1 - Rewind to start of selection
Cursor.SELECTION_END    // 2 - Rewind to end of selection
```

## Input State Modes

```javascript
Cursor.INPUT_STATE_INDEPENDENT      // Default - cursor independent of score
Cursor.INPUT_STATE_SYNC_WITH_SCORE  // Cursor synced with score input state
```

## Navigation Methods

### rewind(mode)
Move cursor to beginning of score or selection.

```javascript
cursor.rewind(Cursor.SCORE_START);       // Go to beginning
cursor.rewind(Cursor.SELECTION_START);   // Go to selection start
cursor.rewind(Cursor.SELECTION_END);     // Go to selection end
```

### rewindToTick(tick)
Move cursor to specific tick position.

```javascript
cursor.rewindToTick(480);  // Move to tick 480
```

### rewindToFraction(fraction)
Move cursor to specific time position.

```javascript
var f = fraction(1, 1);  // Create fraction for 1 whole note
cursor.rewindToFraction(f);
```

### next()
Move to next segment (returns false if at end).

```javascript
while (cursor.next()) {
    // Process each segment
    var el = cursor.element;
}
```

### nextMeasure()
Move to next measure (returns false if at end).

```javascript
while (cursor.nextMeasure()) {
    // Process each measure
}
```

### prev()
Move to previous segment (returns false if at start).

```javascript
cursor.prev();
```

### time(includeRepeats)
Get time position in seconds.

```javascript
var timeInSeconds = cursor.time(false);
var timeWithRepeats = cursor.time(true);
```

## Note Input Methods

### setDuration(z, n)
Set the duration for subsequent notes/rests as a fraction z/n.

```javascript
// Common durations:
cursor.setDuration(1, 1);   // Whole note
cursor.setDuration(1, 2);   // Half note
cursor.setDuration(1, 4);   // Quarter note
cursor.setDuration(1, 8);   // Eighth note
cursor.setDuration(1, 16);  // Sixteenth note
cursor.setDuration(1, 32);  // 32nd note
cursor.setDuration(1, 64);  // 64th note

// Dotted notes (multiply by 3/2):
cursor.setDuration(3, 8);   // Dotted quarter (3/8 = 1/4 * 3/2)
cursor.setDuration(3, 16);  // Dotted eighth
```

### addNote(pitch, addToChord)
Add a note at the cursor position.

**Parameters:**
- `pitch` (int): MIDI pitch value (0-127). Middle C = 60.
- `addToChord` (bool, optional): If true, add to existing chord. Default: false.

```javascript
// Add single notes
cursor.addNote(60);           // Add Middle C
cursor.addNote(64);           // Add E above middle C
cursor.addNote(67);           // Add G

// Build a chord
cursor.addNote(60, false);    // C (new chord)
cursor.addNote(64, true);     // Add E to chord
cursor.addNote(67, true);     // Add G to chord (C major chord)
```

### addRest()
Add a rest at the cursor position with the current duration.

```javascript
cursor.setDuration(1, 4);
cursor.addRest();  // Quarter rest
```

### addTuplet(ratio, duration)
Add a tuplet at the cursor position.

**Parameters:**
- `ratio`: FractionWrapper - The tuplet ratio (e.g., 3/2 for triplet)
- `duration`: FractionWrapper - The total duration

```javascript
var ratio = fraction(3, 2);      // Triplet: 3 notes in space of 2
var duration = fraction(1, 4);   // Quarter note duration
cursor.addTuplet(ratio, duration);
```

### add(element)
Add an arbitrary element at the cursor position.

```javascript
var text = newElement(Element.STAFF_TEXT);
text.text = "Hello";
cursor.add(text);
```

## Complete Example: Adding a C Major Scale

```javascript
function addCMajorScale() {
    var cursor = curScore.newCursor();
    cursor.rewind(Cursor.SCORE_START);
    cursor.voice = 0;
    cursor.staffIdx = 0;

    curScore.startCmd("Add C Major Scale");

    cursor.setDuration(1, 4);  // Quarter notes

    // C Major scale: C D E F G A B C
    var pitches = [60, 62, 64, 65, 67, 69, 71, 72];

    for (var i = 0; i < pitches.length; i++) {
        cursor.addNote(pitches[i]);
    }

    curScore.endCmd();
}
```

## Complete Example: Adding a Chord Progression

```javascript
function addChordProgression() {
    var cursor = curScore.newCursor();
    cursor.rewind(Cursor.SCORE_START);

    curScore.startCmd("Add Chord Progression");

    cursor.setDuration(1, 1);  // Whole notes

    // C major chord
    cursor.addNote(60, false);  // C
    cursor.addNote(64, true);   // E
    cursor.addNote(67, true);   // G

    // F major chord
    cursor.addNote(65, false);  // F
    cursor.addNote(69, true);   // A
    cursor.addNote(72, true);   // C

    // G major chord
    cursor.addNote(67, false);  // G
    cursor.addNote(71, true);   // B
    cursor.addNote(74, true);   // D

    // C major chord (return)
    cursor.addNote(60, false);  // C
    cursor.addNote(64, true);   // E
    cursor.addNote(67, true);   // G

    curScore.endCmd();
}
```

## Iterating Through Score Elements

```javascript
function iterateScore() {
    var cursor = curScore.newCursor();
    cursor.rewind(Cursor.SCORE_START);

    while (cursor.segment) {
        var element = cursor.element;
        if (element) {
            if (element.type == Element.CHORD) {
                var chord = element;
                var notes = chord.notes;
                for (var i = 0; i < notes.length; i++) {
                    console.log("Note pitch: " + notes[i].pitch);
                }
            } else if (element.type == Element.REST) {
                console.log("Found rest");
            }
        }
        cursor.next();
    }
}
```
