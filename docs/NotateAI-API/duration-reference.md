# Duration Reference

This document describes how note/rest durations are represented in MuseScore, including tick values and the `setDuration()` method.

## Source File
`docs/ticklength.md`

## Duration Basics

Durations in MuseScore are expressed as fractions of a whole note. The Cursor's `setDuration(z, n)` method takes a fraction z/n.

## Common Duration Values

### Standard Note Values

| Duration | Fraction | setDuration() |
|----------|----------|---------------|
| Double whole (breve) | 2/1 | `setDuration(2, 1)` |
| Whole (semibreve) | 1/1 | `setDuration(1, 1)` |
| Half (minim) | 1/2 | `setDuration(1, 2)` |
| Quarter (crotchet) | 1/4 | `setDuration(1, 4)` |
| Eighth (quaver) | 1/8 | `setDuration(1, 8)` |
| 16th (semiquaver) | 1/16 | `setDuration(1, 16)` |
| 32nd (demisemiquaver) | 1/32 | `setDuration(1, 32)` |
| 64th (hemidemisemiquaver) | 1/64 | `setDuration(1, 64)` |
| 128th | 1/128 | `setDuration(1, 128)` |
| 256th | 1/256 | `setDuration(1, 256)` |

### Dotted Note Values

A dot adds half the value. The formula is: `duration * 3/2`

| Duration | Fraction | setDuration() |
|----------|----------|---------------|
| Dotted whole | 3/2 | `setDuration(3, 2)` |
| Dotted half | 3/4 | `setDuration(3, 4)` |
| Dotted quarter | 3/8 | `setDuration(3, 8)` |
| Dotted eighth | 3/16 | `setDuration(3, 16)` |
| Dotted 16th | 3/32 | `setDuration(3, 32)` |
| Dotted 32nd | 3/64 | `setDuration(3, 64)` |

### Double-Dotted Note Values

Double dot adds 3/4 of the original value. Formula: `duration * 7/4`

| Duration | Fraction | setDuration() |
|----------|----------|---------------|
| Double-dotted whole | 7/4 | `setDuration(7, 4)` |
| Double-dotted half | 7/8 | `setDuration(7, 8)` |
| Double-dotted quarter | 7/16 | `setDuration(7, 16)` |
| Double-dotted eighth | 7/32 | `setDuration(7, 32)` |

## Tick Values

Internally, MuseScore uses "ticks" to measure time. The number of ticks per quarter note is defined by the `division` property (typically 480).

### Tick Length Table

| Duration | Ticks (division=480) | Formula |
|----------|---------------------|---------|
| Whole note | 1920 | `4 * division` |
| Double-dotted half | 1680 | `3.5 * division` |
| Dotted half | 1440 | `3 * division` |
| Triplet whole (1/3 breve) | 1280 | `8 * division / 3` |
| Half note | 960 | `2 * division` |
| Double-dotted quarter | 840 | `1.75 * division` |
| Dotted quarter | 720 | `1.5 * division` |
| Triplet half (1/3 whole) | 640 | `4 * division / 3` |
| Quarter note | 480 | `division` |
| Double-dotted eighth | 420 | `0.875 * division` |
| Dotted eighth | 360 | `0.75 * division` |
| Triplet quarter (1/3 half) | 320 | `2 * division / 3` |
| Eighth note | 240 | `division / 2` |
| Double-dotted 16th | 210 | `0.4375 * division` |
| Dotted 16th | 180 | `0.375 * division` |
| Triplet eighth (1/3 quarter) | 160 | `division / 3` |
| 16th note | 120 | `division / 4` |
| Double-dotted 32nd | 105 | `0.21875 * division` |
| Dotted 32nd | 90 | `0.1875 * division` |
| Triplet 16th (1/3 eighth) | 80 | `division / 6` |
| 32nd note | 60 | `division / 8` |
| Dotted 64th | 45 | `0.09375 * division` |
| Triplet 32nd (1/3 16th) | 40 | `division / 12` |
| 64th note | 30 | `division / 16` |

## Working with Duration

### Setting Duration via Cursor

```javascript
var cursor = curScore.newCursor();
cursor.rewind(Cursor.SCORE_START);

// Standard durations
cursor.setDuration(1, 4);    // Quarter note
cursor.addNote(60);

cursor.setDuration(1, 8);    // Eighth note
cursor.addNote(62);

// Dotted duration
cursor.setDuration(3, 8);    // Dotted quarter
cursor.addNote(64);
```

### Duration via Pad Actions

You can also set duration using pad actions:

```javascript
cmd("pad-note-4");    // Set quarter note duration
cmd("note-c");        // Enter C with quarter duration

cmd("pad-note-8");    // Set eighth note duration
cmd("pad-dot");       // Add dot (now dotted eighth)
cmd("note-d");        // Enter D with dotted eighth duration
```

### Reading Duration from Elements

```javascript
if (cursor.element && cursor.element.type == Element.CHORD) {
    var chord = cursor.element;
    var duration = chord.duration;    // Fraction object
    console.log("Duration: " + duration.numerator + "/" + duration.denominator);
}
```

## Tuplets

Tuplets allow non-standard divisions of time.

### Common Tuplet Ratios

| Tuplet | Ratio | Description |
|--------|-------|-------------|
| Duplet | 2:3 | 2 notes in time of 3 |
| Triplet | 3:2 | 3 notes in time of 2 |
| Quadruplet | 4:3 | 4 notes in time of 3 |
| Quintuplet | 5:4 | 5 notes in time of 4 |
| Sextuplet | 6:4 | 6 notes in time of 4 |
| Septuplet | 7:4 | 7 notes in time of 4 |
| Octuplet | 8:6 | 8 notes in time of 6 |
| Nonuplet | 9:8 | 9 notes in time of 8 |

### Creating Tuplets via Action

```javascript
// Enter triplet mode and add notes
cmd("triplet");       // Start triplet
cmd("note-c");        // First triplet note
cmd("note-d");        // Second triplet note
cmd("note-e");        // Third triplet note (completes triplet)
```

### Creating Tuplets via Cursor

```javascript
cursor.setDuration(1, 4);  // Quarter note base duration
var ratio = fraction(3, 2);          // 3:2 ratio (triplet)
var duration = fraction(1, 4);       // Total duration of tuplet

curScore.startCmd("Add Triplet");
cursor.addTuplet(ratio, duration);

// Now add the notes inside the tuplet
cursor.setDuration(1, 8);  // Eighth notes inside triplet
cursor.addNote(60);
cursor.addNote(62);
cursor.addNote(64);
curScore.endCmd();
```

## Duration Types (DurationType.*)

| Type | Description |
|------|-------------|
| `DurationType.V_LONG` | Longa |
| `DurationType.V_BREVE` | Breve (double whole) |
| `DurationType.V_WHOLE` | Whole note |
| `DurationType.V_HALF` | Half note |
| `DurationType.V_QUARTER` | Quarter note |
| `DurationType.V_EIGHTH` | Eighth note |
| `DurationType.V_16TH` | 16th note |
| `DurationType.V_32ND` | 32nd note |
| `DurationType.V_64TH` | 64th note |
| `DurationType.V_128TH` | 128th note |
| `DurationType.V_256TH` | 256th note |
| `DurationType.V_512TH` | 512th note |
| `DurationType.V_1024TH` | 1024th note |

## Time Signatures

Time signatures affect how durations are grouped and beamed.

### Common Time Signatures

| Time Sig | Beats | Beat Unit | Bar Duration |
|----------|-------|-----------|--------------|
| 4/4 | 4 | Quarter | 1 whole note |
| 3/4 | 3 | Quarter | 3/4 whole note |
| 2/4 | 2 | Quarter | 1/2 whole note |
| 6/8 | 6 | Eighth | 3/4 whole note |
| 2/2 | 2 | Half | 1 whole note |
| 3/8 | 3 | Eighth | 3/8 whole note |
| 9/8 | 9 | Eighth | 9/8 whole note |
| 12/8 | 12 | Eighth | 3/2 whole note |

### Working with Time Signatures

```javascript
// Get time signature at cursor
var measure = cursor.measure;
var timeSig = measure.timesig;
console.log("Time signature: " + timeSig.numerator + "/" + timeSig.denominator);
```

## Practical Examples

### Creating a Rhythm Pattern

```javascript
function addRhythm(cursor) {
    curScore.startCmd("Add Rhythm");

    // Quarter note
    cursor.setDuration(1, 4);
    cursor.addNote(60);

    // Two eighth notes
    cursor.setDuration(1, 8);
    cursor.addNote(62);
    cursor.addNote(64);

    // Dotted quarter + eighth
    cursor.setDuration(3, 8);
    cursor.addNote(65);
    cursor.setDuration(1, 8);
    cursor.addNote(67);

    curScore.endCmd();
}
```

### Creating a Compound Meter Pattern

```javascript
function add6_8Pattern(cursor) {
    curScore.startCmd("Add 6/8 Pattern");

    cursor.setDuration(1, 8);

    // First beat group (3 eighths)
    cursor.addNote(60);
    cursor.addNote(62);
    cursor.addNote(64);

    // Second beat group (3 eighths)
    cursor.addNote(65);
    cursor.addNote(67);
    cursor.addNote(69);

    curScore.endCmd();
}
```

### Filling a Measure with Rests

```javascript
function fillWithRests(cursor, timeSigNumerator, timeSigDenominator) {
    curScore.startCmd("Fill with Rests");

    // Calculate number of rest units needed
    cursor.setDuration(1, timeSigDenominator);

    for (var i = 0; i < timeSigNumerator; i++) {
        cursor.addRest();
    }

    curScore.endCmd();
}

// Fill a 4/4 measure
fillWithRests(cursor, 4, 4);
```

## Helper Functions

```javascript
// Convert BPM and duration to seconds
function durationToSeconds(durationFraction, bpm) {
    var beatsPerSecond = bpm / 60;
    var quarterNotesPerSecond = beatsPerSecond;  // Assuming quarter = beat
    var wholeNotesPerSecond = quarterNotesPerSecond / 4;
    var duration = durationFraction.numerator / durationFraction.denominator;
    return duration / wholeNotesPerSecond;
}

// Get note duration as string description
function durationToString(z, n) {
    var durations = {
        '2/1': 'breve',
        '1/1': 'whole',
        '1/2': 'half',
        '1/4': 'quarter',
        '1/8': 'eighth',
        '1/16': '16th',
        '1/32': '32nd',
        '1/64': '64th',
        '3/2': 'dotted whole',
        '3/4': 'dotted half',
        '3/8': 'dotted quarter',
        '3/16': 'dotted eighth',
        '3/32': 'dotted 16th'
    };
    var key = z + '/' + n;
    return durations[key] || (z + '/' + n);
}
```
