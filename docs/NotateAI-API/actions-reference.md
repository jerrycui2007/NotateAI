# Actions Reference

Actions are command codes that can be dispatched to perform operations in MuseScore. They can be invoked via the `cmd()` function.

## Source File
`src/notation/internal/notationuiactions.cpp`

## Using Actions

```javascript
// Via MuseScore QML API
cmd("action-code");

// Example
cmd("note-input");  // Toggle note input mode
cmd("note-c");      // Enter note C
```

## Navigation Actions

### Score Navigation

| Action Code | Description |
|-------------|-------------|
| `next-element` | Select next element in score |
| `prev-element` | Select previous element in score |
| `notation-move-right` | Next chord / shift text right |
| `notation-move-left` | Previous chord / shift text left |
| `notation-move-right-quickly` | Next measure / shift text right quickly |
| `notation-move-left-quickly` | Previous measure / shift text left quickly |
| `first-element` | Go to first element in score |
| `last-element` | Go to last element in score |

### Chord Navigation

| Action Code | Description |
|-------------|-------------|
| `up-chord` | Select note/rest above |
| `down-chord` | Select note/rest below |
| `top-chord` | Select top note in chord |
| `bottom-chord` | Select bottom note in chord |

### Staff/Voice Navigation

| Action Code | Description |
|-------------|-------------|
| `move-up` | Move to staff above |
| `move-down` | Move to staff below |
| `next-track` | Go to next staff or voice |
| `prev-track` | Go to previous staff or voice |

### Frame/System Navigation

| Action Code | Description |
|-------------|-------------|
| `next-frame` | Go to next frame |
| `prev-frame` | Go to previous frame |
| `next-system` | Go to next system |
| `prev-system` | Go to previous system |

## Note Input Actions

### Note Input Mode

| Action Code | Description |
|-------------|-------------|
| `note-input` | Toggle note input mode |
| `note-input-steptime` | Step-time note input |
| `note-input-rhythm` | Rhythm note input |
| `note-input-repitch` | Re-pitch note input |
| `note-input-realtime-auto` | Real-time (automatic) input |
| `note-input-realtime-manual` | Real-time (manual) input |
| `note-input-timewise` | Timewise (insert) input |

### Enter Notes (by name)

| Action Code | Description |
|-------------|-------------|
| `note-c` | Enter note C |
| `note-d` | Enter note D |
| `note-e` | Enter note E |
| `note-f` | Enter note F |
| `note-g` | Enter note G |
| `note-a` | Enter note A |
| `note-b` | Enter note B |

### Add Notes to Chord

| Action Code | Description |
|-------------|-------------|
| `chord-c` | Add C to chord |
| `chord-d` | Add D to chord |
| `chord-e` | Add E to chord |
| `chord-f` | Add F to chord |
| `chord-g` | Add G to chord |
| `chord-a` | Add A to chord |
| `chord-b` | Add B to chord |

### Insert Notes

| Action Code | Description |
|-------------|-------------|
| `insert-c` | Insert C |
| `insert-d` | Insert D |
| `insert-e` | Insert E |
| `insert-f` | Insert F |
| `insert-g` | Insert G |
| `insert-a` | Insert A |
| `insert-b` | Insert B |

### Rests

| Action Code | Description |
|-------------|-------------|
| `rest` | Enter rest |
| `rest-TAB` | Enter rest (tablature) |

### Duration

| Action Code | Description |
|-------------|-------------|
| `pad-note-1` | Set duration: whole note |
| `pad-note-2` | Set duration: half note |
| `pad-note-4` | Set duration: quarter note |
| `pad-note-8` | Set duration: eighth note |
| `pad-note-16` | Set duration: 16th note |
| `pad-note-32` | Set duration: 32nd note |
| `pad-note-64` | Set duration: 64th note |
| `pad-note-128` | Set duration: 128th note |
| `pad-note-256` | Set duration: 256th note |
| `pad-note-512` | Set duration: 512th note |
| `pad-note-1024` | Set duration: 1024th note |
| `pad-dot` | Toggle dot |
| `pad-dotdot` | Toggle double dot |
| `pad-dot3` | Toggle triple dot |
| `pad-dot4` | Toggle quadruple dot |
| `double-duration` | Double selected duration |
| `half-duration` | Halve selected duration |
| `inc-duration-dotted` | Double duration (dotted) |
| `dec-duration-dotted` | Halve duration (dotted) |

### Tablature Frets

| Action Code | Description |
|-------------|-------------|
| `fret-0` through `fret-14` | Enter TAB fret 0-14 |

## Pitch Modification Actions

| Action Code | Description |
|-------------|-------------|
| `pitch-up` | Move pitch up (semitone) |
| `pitch-down` | Move pitch down (semitone) |
| `pitch-up-octave` | Move pitch up an octave |
| `pitch-down-octave` | Move pitch down an octave |
| `pitch-up-diatonic` | Move pitch up (diatonic) |
| `pitch-down-diatonic` | Move pitch down (diatonic) |

## Accidentals

| Action Code | Description |
|-------------|-------------|
| `flat` | Add flat |
| `flat2` | Add double flat |
| `sharp` | Add sharp |
| `sharp2` | Add double sharp |
| `natural` | Add natural |

## Intervals

| Action Code | Description |
|-------------|-------------|
| `interval1` | Enter unison |
| `interval2` | Enter second above |
| `interval3` | Enter third above |
| `interval4` | Enter fourth above |
| `interval5` | Enter fifth above |
| `interval6` | Enter sixth above |
| `interval7` | Enter seventh above |
| `interval8` | Enter octave above |
| `interval9` | Enter ninth above |
| `interval10` | Enter tenth above |
| `interval-2` | Enter second below |
| `interval-3` | Enter third below |
| `interval-4` | Enter fourth below |
| `interval-5` | Enter fifth below |
| `interval-6` | Enter sixth below |
| `interval-7` | Enter seventh below |
| `interval-8` | Enter octave below |
| `interval-9` | Enter ninth below |
| `interval-10` | Enter tenth below |

## Voice Actions

| Action Code | Description |
|-------------|-------------|
| `voice-1` | Enter notes in voice 1 |
| `voice-2` | Enter notes in voice 2 |
| `voice-3` | Enter notes in voice 3 |
| `voice-4` | Enter notes in voice 4 |
| `voice-x12` | Exchange voice 1-2 |
| `voice-x13` | Exchange voice 1-3 |
| `voice-x14` | Exchange voice 1-4 |
| `voice-x23` | Exchange voice 2-3 |
| `voice-x24` | Exchange voice 2-4 |
| `voice-x34` | Exchange voice 3-4 |

## Tuplets

| Action Code | Description |
|-------------|-------------|
| `duplet` | Enter duplet |
| `triplet` | Enter triplet |
| `quadruplet` | Enter quadruplet |
| `quintuplet` | Enter quintuplet |
| `sextuplet` | Enter sextuplet |
| `septuplet` | Enter septuplet |
| `octuplet` | Enter octuplet |
| `nonuplet` | Enter nonuplet |
| `tuplet-dialog` | Open tuplet dialog |

## Grace Notes

| Action Code | Description |
|-------------|-------------|
| `acciaccatura` | Add acciaccatura |
| `appoggiatura` | Add appoggiatura |
| `grace4` | Add grace note: quarter |
| `grace16` | Add grace note: 16th |
| `grace32` | Add grace note: 32nd |
| `grace8after` | Add grace note: 8th after |
| `grace16after` | Add grace note: 16th after |
| `grace32after` | Add grace note: 32nd after |

## Articulations and Ornaments

| Action Code | Description |
|-------------|-------------|
| `add-marcato` | Add marcato |
| `add-sforzato` | Add sforzato (accent) |
| `add-tenuto` | Add tenuto |
| `add-staccato` | Add staccato |
| `toggle-visible` | Toggle element visibility |

## Ties and Slurs

| Action Code | Description |
|-------------|-------------|
| `tie` | Add tie |
| `chord-tie` | Add tied note to chord |
| `add-slur` | Add slur |
| `add-laissez-vib` | Add laissez vibrer |
| `add-hammer-on-pull-off` | Add hammer-on/pull-off |

## Lines and Spanners

| Action Code | Description |
|-------------|-------------|
| `add-8va` | Add ottava 8va alta |
| `add-8vb` | Add ottava 8va bassa |
| `add-hairpin` | Add crescendo |
| `add-hairpin-reverse` | Add diminuendo |
| `add-noteline` | Add note-anchored line |

## Text Elements

| Action Code | Description |
|-------------|-------------|
| `title-text` | Add title |
| `subtitle-text` | Add subtitle |
| `composer-text` | Add composer |
| `poet-text` | Add lyricist |
| `part-text` | Add part name |
| `system-text` | Add system text |
| `staff-text` | Add staff text |
| `expression-text` | Add expression text |
| `rehearsalmark-text` | Add rehearsal mark |
| `instrument-change-text` | Add instrument change |
| `fingering-text` | Add fingering |
| `sticking-text` | Add sticking |
| `chord-text` | Add chord symbol |
| `roman-numeral-text` | Add Roman numeral analysis |
| `nashville-number-text` | Add Nashville number |
| `lyrics` | Add lyrics |
| `figured-bass` | Add figured bass |
| `tempo` | Add tempo marking |
| `add-dynamic` | Add dynamic |

## Measures and Structure

| Action Code | Description |
|-------------|-------------|
| `insert-measure` | Insert one measure before selection |
| `insert-measures` | Insert measures before selection |
| `append-measure` | Insert one measure at end |
| `append-measures` | Insert measures at end |
| `time-delete` | Delete selected measures |
| `split-measure` | Split measure before selected note |
| `join-measures` | Join selected measures |

## Frames

| Action Code | Description |
|-------------|-------------|
| `insert-hbox` | Insert horizontal frame |
| `insert-vbox` | Insert vertical frame |
| `insert-textframe` | Insert text frame |
| `insert-fretframe` | Insert fretboard diagram legend |
| `append-hbox` | Append horizontal frame |
| `append-vbox` | Append vertical frame |
| `append-textframe` | Append text frame |

## Breaks

| Action Code | Description |
|-------------|-------------|
| `system-break` | Add/remove system break |
| `page-break` | Add/remove page break |
| `section-break` | Add/remove section break |
| `apply-system-lock` | Add/remove system lock |

## Beaming

| Action Code | Description |
|-------------|-------------|
| `beam-auto` | Auto beam |
| `beam-none` | No beam |
| `beam-break-left` | Break beam left |
| `beam-break-inner-8th` | Break inner beams (8th) |
| `beam-break-inner-16th` | Break inner beams (16th) |
| `beam-join` | Join beams |
| `beam-feathered-decelerate` | Feathered beam decelerate |
| `beam-feathered-accelerate` | Feathered beam accelerate |

## Selection Actions

| Action Code | Description |
|-------------|-------------|
| `notation-select-all` | Select all |
| `notation-select-section` | Select section |
| `select-similar` | Select similar elements |
| `select-similar-staff` | Select similar on same staff |
| `select-similar-range` | Select similar in range |
| `select-next-chord` | Add next chord to selection |
| `select-prev-chord` | Add previous chord to selection |

## Clipboard Actions

| Action Code | Description |
|-------------|-------------|
| `action://notation/copy` | Copy selection |
| `action://notation/cut` | Cut selection |
| `action://notation/paste` | Paste |
| `action://notation/delete` | Delete selection |
| `notation-paste-half` | Paste half duration |
| `notation-paste-double` | Paste double duration |
| `notation-paste-special` | Paste special |
| `notation-swap` | Swap with clipboard |

## Layout and View

| Action Code | Description |
|-------------|-------------|
| `view-mode-page` | Page view |
| `view-mode-continuous` | Continuous view (horizontal) |
| `view-mode-single` | Continuous view (vertical) |
| `zoomin` | Zoom in |
| `zoomout` | Zoom out |
| `zoom100` | Zoom to 100% |

## Reset Actions

| Action Code | Description |
|-------------|-------------|
| `reset` | Reset shapes and positions |
| `reset-stretch` | Reset layout stretch |
| `reset-beammode` | Reset beams to default |
| `reset-to-default-layout` | Reset entire score to default layout |

## Tools

| Action Code | Description |
|-------------|-------------|
| `transpose` | Open transpose dialog |
| `explode` | Explode (distribute to staves) |
| `implode` | Implode (combine staves) |
| `realize-chord-symbols` | Realize chord symbols |
| `slash-fill` | Fill with slashes |
| `slash-rhythm` | Toggle rhythmic slash notation |
| `pitch-spell` | Optimize enharmonic spelling |
| `reset-groupings` | Regroup rhythms |
| `resequence-rehearsal-marks` | Resequence rehearsal marks |
| `unroll-repeats` | Unroll repeats |

## Undo/Redo

| Action Code | Description |
|-------------|-------------|
| `undo` | Undo last action |
| `redo` | Redo last undone action |

## Legacy/Compatibility Commands

Some legacy commands are mapped to new action codes:

| Legacy Command | Maps To |
|----------------|---------|
| `escape` | `action://notation/cancel` |
| `cut` | `action://notation/cut` |
| `copy` | `action://notation/copy` |
| `paste` | `action://notation/paste` |
| `paste-half` | `notation-paste-half` |
| `paste-double` | `notation-paste-double` |
| `select-all` | `notation-select-all` |
| `delete` | `action://notation/delete` |
| `next-chord` | `notation-move-right` |
| `prev-chord` | `notation-move-left` |
| `prev-measure` | `notation-move-left-quickly` |
