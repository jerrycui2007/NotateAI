# Elements Reference

MuseScore represents musical notation using a hierarchy of elements. This document describes the available element types and their properties.

## Source Files
- `src/engraving/api/v1/elements.h`
- `src/engraving/api/v1/qmlpluginapi.h` (enumerations)

## Creating Elements

```javascript
// Create a new element
var element = newElement(Element.STAFF_TEXT);

// Set properties
element.text = "Hello";

// Add to score via cursor
cursor.add(element);
```

## Element Types (Element.*)

### Note-Related Elements

| Element Type | Description |
|--------------|-------------|
| `Element.NOTE` | A single note |
| `Element.CHORD` | A chord (contains notes) |
| `Element.REST` | A rest |
| `Element.NOTEDOT` | Dot on a note/rest |
| `Element.STEM` | Note stem |
| `Element.HOOK` | Flag on stem |
| `Element.BEAM` | Beam connecting notes |
| `Element.TUPLET` | Tuplet grouping |
| `Element.ACCIDENTAL` | Accidental sign |

### Text Elements

| Element Type | Description |
|--------------|-------------|
| `Element.TEXT` | Generic text |
| `Element.STAFF_TEXT` | Text attached to staff |
| `Element.SYSTEM_TEXT` | Text attached to system |
| `Element.REHEARSAL_MARK` | Rehearsal mark |
| `Element.TEMPO_TEXT` | Tempo marking |
| `Element.LYRICS` | Lyrics |
| `Element.FIGURED_BASS` | Figured bass |
| `Element.HARMONY` | Chord symbol |
| `Element.FINGERING` | Fingering |
| `Element.INSTRUMENT_NAME` | Instrument name |
| `Element.DYNAMIC` | Dynamic marking |
| `Element.EXPRESSION` | Expression text |
| `Element.STICKING` | Sticking notation |

### Lines and Spanners

| Element Type | Description |
|--------------|-------------|
| `Element.SLUR` | Slur |
| `Element.TIE` | Tie |
| `Element.HAIRPIN` | Crescendo/diminuendo hairpin |
| `Element.OTTAVA` | Ottava line |
| `Element.PEDAL` | Pedal marking |
| `Element.TRILL` | Trill line |
| `Element.VIBRATO` | Vibrato line |
| `Element.GLISSANDO` | Glissando |
| `Element.VOLTA` | Volta bracket |
| `Element.TEXTLINE` | Generic text line |
| `Element.NOTELINE` | Note-anchored line |
| `Element.LET_RING` | Let ring line |
| `Element.PALM_MUTE` | Palm mute line |
| `Element.LAISSEZ_VIB` | Laissez vibrer |

### Articulations and Ornaments

| Element Type | Description |
|--------------|-------------|
| `Element.ARTICULATION` | Articulation mark |
| `Element.ORNAMENT` | Ornament |
| `Element.FERMATA` | Fermata |
| `Element.ARPEGGIO` | Arpeggio |
| `Element.TREMOLO` | Tremolo |
| `Element.CHORDLINE` | Chord line (fall, doit, etc.) |

### Clefs, Key, Time Signatures

| Element Type | Description |
|--------------|-------------|
| `Element.CLEF` | Clef |
| `Element.KEYSIG` | Key signature |
| `Element.TIMESIG` | Time signature |

### Barlines and Structure

| Element Type | Description |
|--------------|-------------|
| `Element.BAR_LINE` | Barline |
| `Element.MEASURE` | Measure |
| `Element.SEGMENT` | Segment within measure |
| `Element.SYSTEM` | System (line of music) |
| `Element.PAGE` | Page |
| `Element.SPACER` | Spacer element |

### Frames

| Element Type | Description |
|--------------|-------------|
| `Element.HBOX` | Horizontal frame |
| `Element.VBOX` | Vertical frame |
| `Element.TBOX` | Text frame |
| `Element.FBOX` | Fretboard diagram frame |

### Guitar/Fretted Elements

| Element Type | Description |
|--------------|-------------|
| `Element.FRET_DIAGRAM` | Fretboard diagram |
| `Element.GUITAR_BEND` | Guitar bend |
| `Element.BEND` | Bend |
| `Element.FRETBOARD_DIAGRAM` | Fretboard diagram |

### Layout Elements

| Element Type | Description |
|--------------|-------------|
| `Element.LAYOUT_BREAK` | Layout break (system, page, section) |
| `Element.BRACKET` | Staff bracket |
| `Element.SYSTEM_DIVIDER` | System divider |

### Other Elements

| Element Type | Description |
|--------------|-------------|
| `Element.JUMP` | Jump (D.C., D.S., etc.) |
| `Element.MARKER` | Marker (Fine, Coda, etc.) |
| `Element.IMAGE` | Image |
| `Element.BREATH` | Breath mark |

## Common Element Properties

All elements share these base properties:

| Property | Type | Description |
|----------|------|-------------|
| `type` | int | Element type (from Element enum) |
| `name` | QString | Element type name (read only) |
| `parent` | EngravingItem | Parent element |
| `staff` | Staff | Staff this element belongs to |
| `visible` | bool | Whether element is visible |
| `color` | QColor | Element color |
| `selected` | bool | Whether element is selected (read only) |
| `generated` | bool | Whether element is auto-generated (read only) |
| `z` | int | Stacking order |
| `small` | bool | Whether element is cue size |

### Positioning Properties

| Property | Type | Description |
|----------|------|-------------|
| `offset` | QPointF | Offset from reference position (spatium units) |
| `offsetX` | qreal | X-axis offset |
| `offsetY` | qreal | Y-axis offset |
| `posX` | qreal | Reference position X (read only) |
| `posY` | qreal | Reference position Y (read only) |
| `pos` | QPointF | Reference position (read only) |
| `pagePos` | QPointF | Position in page coordinates (read only) |
| `canvasPos` | QPointF | Position in canvas coordinates (read only) |
| `bbox` | QRectF | Bounding box (read only) |

### Staff Properties

| Property | Type | Description |
|----------|------|-------------|
| `staffIdx` | int | Staff index |
| `effectiveStaffIdx` | int | Effective staff index (for system objects) |
| `vStaffIdx` | int | Staff index accounting for cross-staffing |

## Note Properties

| Property | Type | Description |
|----------|------|-------------|
| `pitch` | int | MIDI pitch (0-127) |
| `tpc` | int | Tonal pitch class |
| `tpc1` | int | TPC in concert pitch |
| `tpc2` | int | TPC in transposing pitch |
| `string` | int | String number (for TAB) |
| `fret` | int | Fret number (for TAB) |
| `line` | int | Staff line position |
| `velocity` | int | MIDI velocity |
| `veloType` | int | Velocity type |
| `tuning` | qreal | Tuning offset in cents |
| `ghost` | bool | Ghost note |
| `play` | bool | Whether note is played |
| `headType` | int | Notehead type |
| `headGroup` | int | Notehead group |
| `mirrorHead` | int | Notehead horizontal direction |
| `hasParentheses` | bool | Whether note has parentheses |
| `accidental` | Accidental | Note's accidental |
| `accidentalType` | int | Accidental type |
| `tie` | Tie | Forward tie |
| `tieBack` | Tie | Backward tie |
| `dots` | array | Note dots |
| `elements` | array | Attached elements |
| `playEvents` | array | Play events |

```javascript
// Access note properties
var note = cursor.element.notes[0];
console.log("Pitch: " + note.pitch);
console.log("TPC: " + note.tpc);

// Modify note
curScore.startCmd("Modify Note");
note.pitch = 60;  // Set to Middle C
note.velocity = 80;
curScore.endCmd();
```

## Chord Properties

| Property | Type | Description |
|----------|------|-------------|
| `notes` | array | Array of notes in chord |
| `stem` | Stem | Stem element |
| `hook` | Hook | Hook element |
| `beam` | Beam | Beam element |
| `lyrics` | array | Attached lyrics |
| `graceNotes` | array | Grace notes |
| `duration` | Fraction | Duration value |
| `durationType` | int | Duration type |
| `stemDirection` | int | Stem direction |
| `up` | bool | Whether stem points up |

```javascript
// Access chord properties
if (cursor.element.type == Element.CHORD) {
    var chord = cursor.element;
    var notes = chord.notes;
    for (var i = 0; i < notes.length; i++) {
        console.log("Note " + i + ": pitch=" + notes[i].pitch);
    }
}
```

## Rest Properties

| Property | Type | Description |
|----------|------|-------------|
| `duration` | Fraction | Duration value |
| `durationType` | int | Duration type |
| `dots` | int | Number of dots |

## Text Element Properties

| Property | Type | Description |
|----------|------|-------------|
| `text` | QString | Plain text content |
| `htmlText` | QString | HTML formatted text |
| `fontFace` | QString | Font family |
| `fontSize` | qreal | Font size |
| `fontStyle` | int | Font style flags |
| `placement` | int | Placement (above/below) |
| `align` | int | Text alignment |
| `frameType` | int | Frame type |
| `framePadding` | qreal | Frame padding |
| `frameWidth` | qreal | Frame width |
| `frameRound` | int | Frame corner rounding |
| `frameFgColor` | QColor | Frame foreground color |
| `frameBgColor` | QColor | Frame background color |

```javascript
// Create and configure staff text
var text = newElement(Element.STAFF_TEXT);
text.text = "pizz.";
text.placement = Placement.ABOVE;
text.fontSize = 10;

curScore.startCmd("Add Text");
cursor.add(text);
curScore.endCmd();
```

## Dynamic Properties

| Property | Type | Description |
|----------|------|-------------|
| `dynamicType` | int | Dynamic type (p, mp, mf, f, etc.) |
| `velocity` | int | MIDI velocity |
| `changeSpeed` | int | Speed of dynamic change |

## Tempo Text Properties

| Property | Type | Description |
|----------|------|-------------|
| `tempo` | qreal | Tempo in BPM |
| `tempoFollowText` | bool | Whether tempo follows text |
| `text` | QString | Text content |

## Hairpin Properties

| Property | Type | Description |
|----------|------|-------------|
| `hairpinType` | int | Type (crescendo, diminuendo) |
| `placement` | int | Placement |
| `beginText` | QString | Text at start |
| `continueText` | QString | Continuation text |
| `endText` | QString | Text at end |
| `veloChange` | int | Velocity change |
| `veloChangeSpeed` | int | Speed of change |

## Measure Properties

| Property | Type | Description |
|----------|------|-------------|
| `timesig` | TimeSig | Time signature |
| `startRepeat` | bool | Has start repeat barline |
| `endRepeat` | bool | Has end repeat barline |
| `repeatCount` | int | Repeat count |
| `no` | int | Measure number (0-based) |
| `noString` | QString | Measure number string |
| `irregular` | bool | Is irregular measure |

## Segment Properties

| Property | Type | Description |
|----------|------|-------------|
| `tick` | int | Tick position |
| `segmentType` | int | Segment type |
| `nextInMeasure` | Segment | Next segment in measure |
| `prevInMeasure` | Segment | Previous segment in measure |
| `annotations` | array | Attached annotations |

## Segment Types (Segment.*)

| Type | Description |
|------|-------------|
| `Segment.All` | All segment types |
| `Segment.ChordRest` | Chords and rests |
| `Segment.Clef` | Clef |
| `Segment.KeySig` | Key signature |
| `Segment.TimeSig` | Time signature |
| `Segment.StartRepeatBarLine` | Start repeat |
| `Segment.BarLine` | Barline |
| `Segment.Breath` | Breath |

## Enumerations

### Accidental Types (Accidental.*)

| Type | Description |
|------|-------------|
| `Accidental.NONE` | No accidental |
| `Accidental.FLAT` | Flat |
| `Accidental.NATURAL` | Natural |
| `Accidental.SHARP` | Sharp |
| `Accidental.SHARP2` | Double sharp |
| `Accidental.FLAT2` | Double flat |
| `Accidental.NATURAL_FLAT` | Natural flat |
| `Accidental.NATURAL_SHARP` | Natural sharp |
| `Accidental.SHARP_SHARP` | Sharp sharp |

### Note Head Types (NoteHeadType.*)

| Type | Description |
|------|-------------|
| `NoteHeadType.HEAD_AUTO` | Auto |
| `NoteHeadType.HEAD_WHOLE` | Whole note |
| `NoteHeadType.HEAD_HALF` | Half note |
| `NoteHeadType.HEAD_QUARTER` | Quarter note |
| `NoteHeadType.HEAD_BREVIS` | Brevis |

### Note Head Groups (NoteHeadGroup.*)

| Type | Description |
|------|-------------|
| `NoteHeadGroup.HEAD_NORMAL` | Normal |
| `NoteHeadGroup.HEAD_CROSS` | Cross |
| `NoteHeadGroup.HEAD_PLUS` | Plus |
| `NoteHeadGroup.HEAD_XCIRCLE` | X circle |
| `NoteHeadGroup.HEAD_WITHX` | With X |
| `NoteHeadGroup.HEAD_TRIANGLE_UP` | Triangle up |
| `NoteHeadGroup.HEAD_TRIANGLE_DOWN` | Triangle down |
| `NoteHeadGroup.HEAD_DIAMOND` | Diamond |
| `NoteHeadGroup.HEAD_SLASH` | Slash |
| `NoteHeadGroup.HEAD_DO` | Do |
| `NoteHeadGroup.HEAD_RE` | Re |
| `NoteHeadGroup.HEAD_MI` | Mi |
| `NoteHeadGroup.HEAD_FA` | Fa |
| `NoteHeadGroup.HEAD_SOL` | Sol |
| `NoteHeadGroup.HEAD_LA` | La |
| `NoteHeadGroup.HEAD_TI` | Ti |

### Direction (Direction.*)

| Type | Description |
|------|-------------|
| `Direction.AUTO` | Automatic |
| `Direction.UP` | Up |
| `Direction.DOWN` | Down |

### Placement (Placement.*)

| Type | Description |
|------|-------------|
| `Placement.ABOVE` | Above staff |
| `Placement.BELOW` | Below staff |

### Hairpin Types (HairpinType.*)

| Type | Description |
|------|-------------|
| `HairpinType.CRESC_HAIRPIN` | Crescendo hairpin |
| `HairpinType.DECRESC_HAIRPIN` | Decrescendo hairpin |
| `HairpinType.CRESC_LINE` | Crescendo line |
| `HairpinType.DECRESC_LINE` | Decrescendo line |

### Ottava Types (OttavaType.*)

| Type | Description |
|------|-------------|
| `OttavaType.OTTAVA_8VA` | 8va alta |
| `OttavaType.OTTAVA_8VB` | 8va bassa |
| `OttavaType.OTTAVA_15MA` | 15ma alta |
| `OttavaType.OTTAVA_15MB` | 15ma bassa |
| `OttavaType.OTTAVA_22MA` | 22ma alta |
| `OttavaType.OTTAVA_22MB` | 22ma bassa |

### Beam Modes (Beam.*)

| Type | Description |
|------|-------------|
| `Beam.AUTO` | Auto beaming |
| `Beam.NONE` | No beam |
| `Beam.BEGIN` | Begin beam |
| `Beam.MID` | Middle of beam |
| `Beam.END` | End beam |

### Dynamic Types (DynamicType.*)

| Type | Description |
|------|-------------|
| `DynamicType.OTHER` | Other |
| `DynamicType.PPPPPP` | pppppp |
| `DynamicType.PPPPP` | ppppp |
| `DynamicType.PPPP` | pppp |
| `DynamicType.PPP` | ppp |
| `DynamicType.PP` | pp |
| `DynamicType.P` | p |
| `DynamicType.MP` | mp |
| `DynamicType.MF` | mf |
| `DynamicType.F` | f |
| `DynamicType.FF` | ff |
| `DynamicType.FFF` | fff |
| `DynamicType.FFFF` | ffff |
| `DynamicType.FFFFF` | fffff |
| `DynamicType.FFFFFF` | ffffff |
| `DynamicType.FP` | fp |
| `DynamicType.SF` | sf |
| `DynamicType.SFZ` | sfz |
| `DynamicType.SFF` | sff |
| `DynamicType.SFFZ` | sffz |
| `DynamicType.RFZ` | rfz |
| `DynamicType.RF` | rf |
