# Score API

The Score class represents a musical score and provides methods for score-level manipulation, navigation, and undo/redo management.

## Source File
`src/engraving/api/v1/score.h`

## Accessing the Current Score

```javascript
// From plugin context
var score = curScore;

// Check if score exists
if (!curScore) {
    console.log("No score open");
    return;
}
```

## Properties

### Metadata Properties

| Property | Type | Description |
|----------|------|-------------|
| `composer` | QString | Composer from score properties (read only) |
| `lyricist` | QString | Lyricist from score properties (read only) |
| `title` | QString | Title from workTitle property (read only) |
| `scoreName` | QString | Name of the score file (read/write) |
| `mscoreVersion` | QString | MuseScore version that saved the score (read only) |
| `mscoreRevision` | QString | MuseScore revision (read only) |

### Score Structure Properties

| Property | Type | Description |
|----------|------|-------------|
| `nmeasures` | int | Number of measures (read only) |
| `npages` | int | Number of pages (read only) |
| `nstaves` | int | Number of staves (read only) |
| `ntracks` | int | Number of tracks (nstaves * 4) (read only) |
| `duration` | int | Duration in seconds (read only) |

### Navigation Properties

| Property | Type | Description |
|----------|------|-------------|
| `firstMeasure` | Measure | First measure (read only) |
| `lastMeasure` | Measure | Last measure (read only) |
| `firstMeasureMM` | Measure | First multimeasure rest measure (read only) |
| `lastMeasureMM` | Measure | Last multimeasure rest measure (read only) |
| `lastSegment` | Segment | Last segment (read only) |

### Other Properties

| Property | Type | Description |
|----------|------|-------------|
| `selection` | Selection | Current selection (read only) |
| `style` | MStyle | Style settings (read only). Since 3.5. |
| `parts` | QQmlListProperty<Part> | List of parts |
| `excerpts` | QQmlListProperty<Excerpt> | List of excerpts/linked parts |
| `staves` | QQmlListProperty<Staff> | List of staves. Since 3.6.3. |
| `pages` | QQmlListProperty<Page> | List of pages. Since 4.6. |
| `systems` | QQmlListProperty<System> | List of systems. Since 4.6. |
| `pageNumberOffset` | int | Page numbering offset. Since 3.5. |

### Display Properties (Since 4.6)

| Property | Type | Description |
|----------|------|-------------|
| `layoutMode` | int | Current layout mode |
| `showVerticalFrames` | bool | Show vertical frames |
| `showInvisible` | bool | Show invisible elements |
| `showUnprintable` | bool | Show formatting elements |
| `showFrames` | bool | Show frames |
| `showPageborders` | bool | Show page borders |
| `showSoundFlags` | bool | Show sound flags |
| `markIrregularMeasures` | bool | Mark corrupted measures |
| `showInstrumentNames` | bool | Show instrument names |

### Content Properties

| Property | Type | Description |
|----------|------|-------------|
| `harmonyCount` | int | Number of chord symbols (read only) |
| `hasHarmonies` | bool | Whether score has chord symbols (read only) |
| `hasLyrics` | bool | Whether score has lyrics (read only) |
| `lyricCount` | int | Number of lyric syllables (read only) |
| `keysig` | int | Key signature at start (read only) |

## Methods

### Undo/Redo Management

**IMPORTANT**: Always wrap score modifications in `startCmd()`/`endCmd()` for proper undo support.

#### startCmd(actionName)
Start an undoable command block.

```javascript
curScore.startCmd("My Action Name");  // Name appears in Undo menu
// ... make modifications ...
curScore.endCmd();
```

#### endCmd(rollback)
End an undoable command block.

**Parameters:**
- `rollback` (bool, optional): If true, reverts all changes. Default: false.

```javascript
curScore.endCmd();       // Commit changes
curScore.endCmd(true);   // Rollback all changes since startCmd()
```

### Navigation Methods

#### newCursor()
Create a new cursor for navigating and editing the score.

```javascript
var cursor = curScore.newCursor();
```

#### firstSegment(segmentType)
Get the first segment of a given type.

**Parameters:**
- `segmentType` (int, optional): Segment type bitmask. Default: all types.

```javascript
var seg = curScore.firstSegment();  // First segment of any type
var seg = curScore.firstSegment(Segment.ChordRest);  // First chord/rest
```

#### tick2measure(tick)
Get the measure at a given tick.

**Parameters:**
- `tick`: FractionWrapper - The tick position

```javascript
var measure = curScore.tick2measure(fraction(1, 1));  // Measure at tick
```

#### findSegmentAtTick(types, tick)
Find a segment of given type at a tick.

```javascript
var seg = curScore.findSegmentAtTick(Segment.ChordRest, fraction(0, 1));
```

### Score Modification Methods

#### appendMeasures(n)
Append n measures to the end of the score.

```javascript
curScore.startCmd("Add Measures");
curScore.appendMeasures(4);  // Add 4 measures
curScore.endCmd();
```

#### appendPart(instrumentId)
Add a part with the specified instrument.

**Parameters:**
- `instrumentId` (QString): Instrument ID from instruments.xml

```javascript
curScore.startCmd("Add Part");
curScore.appendPart("voice.soprano");  // Add soprano voice
curScore.appendPart("keyboard.piano"); // Add piano
curScore.endCmd();
```

#### appendPartByMusicXmlId(musicXmlId)
Add a part using MusicXML Sound ID.

```javascript
curScore.appendPartByMusicXmlId("voice.soprano");
```

#### addText(type, text)
Add text to the score.

```javascript
curScore.startCmd("Add Text");
curScore.addText("title", "My Song Title");
curScore.addText("subtitle", "A subtitle");
curScore.addText("composer", "J. S. Bach");
curScore.endCmd();
```

#### addRemoveSystemLocks(interval, lock)
Add or remove system locks at intervals.

**Parameters:**
- `interval` (int): Measure interval for locks
- `lock` (bool): true to add locks, false to remove

```javascript
curScore.addRemoveSystemLocks(4, true);  // Lock every 4 measures
```

#### makeIntoSystem(first, last)
Create a locked system from MeasureBase objects.

```javascript
curScore.makeIntoSystem(firstMeasure, lastMeasure);
```

### Metadata Methods

#### metaTag(tag)
Get a metadata tag value.

```javascript
var title = curScore.metaTag("workTitle");
var composer = curScore.metaTag("composer");
```

#### setMetaTag(tag, val)
Set a metadata tag value.

```javascript
curScore.startCmd("Set Metadata");
curScore.setMetaTag("workTitle", "New Title");
curScore.setMetaTag("composer", "New Composer");
curScore.endCmd();
```

### Utility Methods

#### extractLyrics()
Extract all lyrics as a single string.

```javascript
var lyrics = curScore.extractLyrics();
console.log(lyrics);
```

#### createPlayEvents()
Create play events for all notes based on ornamentation.

```javascript
curScore.createPlayEvents();
```

#### doLayout(startTick, endTick)
Force layout of the score in a tick range.

```javascript
// Layout entire score
curScore.doLayout(fraction(0, 1), fraction(-1, 1));
```

#### showElementInScore(element, staffIdx)
Scroll to show an element in the view.

```javascript
curScore.showElementInScore(element, 0);  // Show on staff 0
```

## Working with Selection

```javascript
var sel = curScore.selection;

// Check if selection is a range
if (sel.isRange) {
    var startSeg = sel.startSegment;
    var endSeg = sel.endSegment;
    var startStaff = sel.startStaff;
    var endStaff = sel.endStaff;
}

// Get selected elements
var elements = sel.elements;
for (var i = 0; i < elements.length; i++) {
    console.log("Element type: " + elements[i].type);
}

// Select an element
sel.select(element, false);  // Replace selection
sel.select(element, true);   // Add to selection

// Select a range
sel.selectRange(startTick, endTick, startStaff, endStaff);

// Clear selection
sel.clear();
```

## Working with Parts

```javascript
var parts = curScore.parts;
for (var i = 0; i < parts.length; i++) {
    var part = parts[i];
    console.log("Part: " + part.partName);
    console.log("Instrument: " + part.instrumentId);
}
```

## Complete Example: Creating a New Score Section

```javascript
function addNewSection() {
    if (!curScore) return;

    curScore.startCmd("Add Section");

    // Add 8 measures
    curScore.appendMeasures(8);

    // Get cursor and navigate to new measures
    var cursor = curScore.newCursor();
    cursor.rewind(Cursor.SCORE_START);

    // Navigate to the new section (skip existing measures)
    var existingMeasures = curScore.nmeasures - 8;
    for (var i = 0; i < existingMeasures; i++) {
        cursor.nextMeasure();
    }

    // Add some notes
    cursor.setDuration(1, 4);
    cursor.addNote(60);  // C
    cursor.addNote(62);  // D
    cursor.addNote(64);  // E
    cursor.addNote(65);  // F

    curScore.endCmd();
}
```

## Complete Example: Modifying All Notes in Score

```javascript
function transposeAllNotesUp() {
    if (!curScore) return;

    curScore.startCmd("Transpose Up");

    var cursor = curScore.newCursor();
    cursor.rewind(Cursor.SCORE_START);

    while (cursor.segment) {
        var element = cursor.element;
        if (element && element.type == Element.CHORD) {
            var notes = element.notes;
            for (var i = 0; i < notes.length; i++) {
                notes[i].pitch += 1;  // Transpose up by semitone
            }
        }
        cursor.next();
    }

    curScore.endCmd();
}
```
