# Pitch Reference

This document describes how pitches are represented in MuseScore, including MIDI pitch values and Tonal Pitch Class (TPC) values.

## Source Files
- `docs/pitch.md`
- `docs/tpc.md`

## MIDI Pitch Values

Note pitches are internally expressed with their respective MIDI pitch values (0-127). This corresponds to the absolute height of the note, regardless of enharmonic spelling.

### Pitch Table by Octave

| Note | Oct -1 | Oct 0 | Oct 1 | Oct 2 | Oct 3 | Oct 4 | Oct 5 | Oct 6 | Oct 7 | Oct 8 | Oct 9 |
|------|--------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
| C    | 0      | 12    | 24    | 36    | 48    | **60** | 72    | 84    | 96    | 108   | 120   |
| C#   | 1      | 13    | 25    | 37    | 49    | 61    | 73    | 85    | 97    | 109   | 121   |
| D    | 2      | 14    | 26    | 38    | 50    | 62    | 74    | 86    | 98    | 110   | 122   |
| D#   | 3      | 15    | 27    | 39    | 51    | 63    | 75    | 87    | 99    | 111   | 123   |
| E    | 4      | 16    | 28    | 40    | 52    | 64    | 76    | 88    | 100   | 112   | 124   |
| F    | 5      | 17    | 29    | 41    | 53    | 65    | 77    | 89    | 101   | 113   | 125   |
| F#   | 6      | 18    | 30    | 42    | 54    | 66    | 78    | 90    | 102   | 114   | 126   |
| G    | 7      | 19    | 31    | 43    | 55    | 67    | 79    | 91    | 103   | 115   | 127   |
| G#   | 8      | 20    | 32    | 44    | 56    | 68    | 80    | 92    | 104   | 116   | --    |
| A    | 9      | 21    | 33    | 45    | 57    | 69    | 81    | 93    | 105   | 117   | --    |
| A#   | 10     | 22    | 34    | 46    | 58    | 70    | 82    | 94    | 106   | 118   | --    |
| B    | 11     | 23    | 35    | 47    | 59    | 71    | 83    | 95    | 107   | 119   | --    |

**Note:** Middle C (C4) = **60**

### Common Reference Pitches

| Description | Pitch |
|-------------|-------|
| Middle C (C4) | 60 |
| Concert A (A4) | 69 |
| Lowest piano note (A0) | 21 |
| Highest piano note (C8) | 108 |
| Bass clef middle line (D3) | 50 |
| Treble clef middle line (B4) | 71 |

### Pitch Calculation Formula

```javascript
// Calculate pitch from note name and octave
function pitchFromNote(noteName, octave) {
    var noteOffsets = {
        'C': 0, 'D': 2, 'E': 4, 'F': 5, 'G': 7, 'A': 9, 'B': 11
    };
    return (octave + 1) * 12 + noteOffsets[noteName.toUpperCase()];
}

// Examples:
pitchFromNote('C', 4);  // 60 (Middle C)
pitchFromNote('A', 4);  // 69 (Concert A)
pitchFromNote('G', 3);  // 55

// Convert pitch to note name
function noteFromPitch(pitch) {
    var noteNames = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];
    var octave = Math.floor(pitch / 12) - 1;
    var noteName = noteNames[pitch % 12];
    return noteName + octave;
}

// Examples:
noteFromPitch(60);  // "C4"
noteFromPitch(69);  // "A4"
```

## Tonal Pitch Class (TPC)

The TPC differentiates between notes with the same MIDI pitch but different enharmonic spellings (e.g., C# vs Db).

### TPC Values by Note Name

| TPC | Note | TPC | Note | TPC | Note | TPC | Note | TPC | Note |
|----:|------|----:|------|----:|------|----:|------|----:|------|
| -1  | Fbb  | 6   | Fb   | 13  | F    | 20  | F#   | 27  | F## |
| 0   | Cbb  | 7   | Cb   | 14  | C    | 21  | C#   | 28  | C## |
| 1   | Gbb  | 8   | Gb   | 15  | G    | 22  | G#   | 29  | G## |
| 2   | Dbb  | 9   | Db   | 16  | D    | 23  | D#   | 30  | D## |
| 3   | Abb  | 10  | Ab   | 17  | A    | 24  | A#   | 31  | A## |
| 4   | Ebb  | 11  | Eb   | 18  | E    | 25  | E#   | 32  | E## |
| 5   | Bbb  | 12  | Bb   | 19  | B    | 26  | B#   | 33  | B## |

### TPC Pattern

The TPC follows the circle of fifths pattern:
- Natural notes: 13-19 (F, C, G, D, A, E, B)
- Single flat: 6-12 (Fb, Cb, Gb, Db, Ab, Eb, Bb)
- Double flat: -1 to 5 (Fbb, Cbb, Gbb, Dbb, Abb, Ebb, Bbb)
- Single sharp: 20-26 (F#, C#, G#, D#, A#, E#, B#)
- Double sharp: 27-33 (F##, C##, G##, D##, A##, E##, B##)

### TPC and Pitch Relationship

| Pitch | TPC Options |
|:-----:|-------------|
| 11 (B/Cb) | 19 (B), 7 (Cb), 31 (A##) |
| 10 (A#/Bb) | 24 (A#), 12 (Bb), 0 (Cbb) |
| 9 (A) | 17 (A), 29 (G##), 5 (Bbb) |
| 8 (G#/Ab) | 22 (G#), 10 (Ab) |
| 7 (G) | 15 (G), 27 (F##), 3 (Abb) |
| 6 (F#/Gb) | 20 (F#), 8 (Gb), 32 (E##) |
| 5 (F/E#) | 13 (F), 25 (E#), 1 (Gbb) |
| 4 (E/Fb) | 18 (E), 6 (Fb), 30 (D##) |
| 3 (D#/Eb) | 23 (D#), 11 (Eb), -1 (Fbb) |
| 2 (D) | 16 (D), 28 (C##), 4 (Ebb) |
| 1 (C#/Db) | 21 (C#), 9 (Db), 33 (B##) |
| 0 (C/B#) | 14 (C), 26 (B#), 2 (Dbb) |

### Working with TPC

```javascript
// Access note TPC
var note = cursor.element.notes[0];
console.log("TPC: " + note.tpc);
console.log("TPC1 (concert): " + note.tpc1);
console.log("TPC2 (transposing): " + note.tpc2);

// Common TPC values for reference
var TPC = {
    // Natural notes
    C: 14, D: 16, E: 18, F: 13, G: 15, A: 17, B: 19,

    // Single sharps
    Cs: 21, Ds: 23, Es: 25, Fs: 20, Gs: 22, As: 24, Bs: 26,

    // Single flats
    Cb: 7, Db: 9, Eb: 11, Fb: 6, Gb: 8, Ab: 10, Bb: 12
};
```

## Note Properties

When working with notes, these properties are available:

```javascript
var note = cursor.element.notes[0];

// Read properties
var pitch = note.pitch;      // MIDI pitch (0-127)
var tpc = note.tpc;          // Tonal pitch class
var tpc1 = note.tpc1;        // TPC in concert pitch
var tpc2 = note.tpc2;        // TPC in transposing pitch

// Modify properties
curScore.startCmd("Change Pitch");
note.pitch = 62;             // Change to D
note.tpc1 = 16;              // Set TPC to D natural
note.tpc2 = 16;              // Same for transposing
curScore.endCmd();
```

## Key Signature Values

Key signatures use the number of accidentals:
- Negative values = flats
- Zero = C major / A minor
- Positive values = sharps

| Key Signature | Value |
|---------------|-------|
| C major / A minor | 0 |
| G major / E minor | 1 |
| D major / B minor | 2 |
| A major / F# minor | 3 |
| E major / C# minor | 4 |
| B major / G# minor | 5 |
| F# major / D# minor | 6 |
| C# major / A# minor | 7 |
| F major / D minor | -1 |
| Bb major / G minor | -2 |
| Eb major / C minor | -3 |
| Ab major / F minor | -4 |
| Db major / Bb minor | -5 |
| Gb major / Eb minor | -6 |
| Cb major / Ab minor | -7 |

```javascript
// Get key signature
var keySig = cursor.keySignature;  // Number of sharps/flats

// Key signature names
var keyNames = {
    '-7': 'Cb major',
    '-6': 'Gb major',
    '-5': 'Db major',
    '-4': 'Ab major',
    '-3': 'Eb major',
    '-2': 'Bb major',
    '-1': 'F major',
    '0': 'C major',
    '1': 'G major',
    '2': 'D major',
    '3': 'A major',
    '4': 'E major',
    '5': 'B major',
    '6': 'F# major',
    '7': 'C# major'
};
```

## Example: Transposing Notes

```javascript
function transposeSelection(semitones) {
    var selection = curScore.selection;
    if (!selection) return;

    curScore.startCmd("Transpose");

    var elements = selection.elements;
    for (var i = 0; i < elements.length; i++) {
        var el = elements[i];
        if (el.type == Element.NOTE) {
            el.pitch += semitones;
            // Note: TPC may need adjustment for proper spelling
        }
    }

    curScore.endCmd();
}

// Transpose up a perfect fifth (7 semitones)
transposeSelection(7);

// Transpose down an octave (12 semitones)
transposeSelection(-12);
```

## Example: Building Chords by Pitch

```javascript
// Build a major chord from root pitch
function addMajorChord(cursor, rootPitch) {
    curScore.startCmd("Add Major Chord");
    cursor.addNote(rootPitch, false);      // Root
    cursor.addNote(rootPitch + 4, true);   // Major 3rd (+4 semitones)
    cursor.addNote(rootPitch + 7, true);   // Perfect 5th (+7 semitones)
    curScore.endCmd();
}

// Build a minor chord
function addMinorChord(cursor, rootPitch) {
    curScore.startCmd("Add Minor Chord");
    cursor.addNote(rootPitch, false);      // Root
    cursor.addNote(rootPitch + 3, true);   // Minor 3rd (+3 semitones)
    cursor.addNote(rootPitch + 7, true);   // Perfect 5th (+7 semitones)
    curScore.endCmd();
}

// Build a dominant 7th chord
function addDom7Chord(cursor, rootPitch) {
    curScore.startCmd("Add Dom7 Chord");
    cursor.addNote(rootPitch, false);      // Root
    cursor.addNote(rootPitch + 4, true);   // Major 3rd
    cursor.addNote(rootPitch + 7, true);   // Perfect 5th
    cursor.addNote(rootPitch + 10, true);  // Minor 7th (+10 semitones)
    curScore.endCmd();
}

// Usage
addMajorChord(cursor, 60);  // C major chord
addMinorChord(cursor, 69);  // A minor chord
addDom7Chord(cursor, 55);   // G7 chord
```
