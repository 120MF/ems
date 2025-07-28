# EMS (Embedded Music Sheet) Format Specification v1.0

English Version | [中文版本](format_zh.md)

## Overview

EMS (Embedded Music Sheet) is a lightweight music notation format designed specifically for embedded systems and monophonic instruments. It uses a simple string-based syntax that is human-readable, memory-efficient, and easy to parse on microcontrollers.

## Syntax Structure

```ems
(BPM){default_beat}note_sequence
```

### Components

1. **BPM Section** (Optional): `(120)` - Beats per minute in parentheses
2. **Default Beat** (Optional): `{4}` - Default note duration in curly braces
3. **Note Sequence**: Series of notes with optional modifiers

## Basic Notes

EMS uses numbered musical notation (Jianpu/Cipher notation):

1 = Do (C) 2 = Re (D) 3 = Mi (E) 4 = Fa (F) 5 = Sol (G) 6 = La (A) 7 = Si (B) 0 = Rest/Silence

## Duration Modifiers

Duration modifiers are placed **after** the note number:

'1,' = One beat (default duration) '1-' = Half beat (1/2)

'1.' = Quarter beat (1/4) '1_' = Two beats (2x)

## Octave Modifiers

Octave modifiers use backticks before or after notes:

## Accidentals (Sharps and Flats)

Use letters to indicate semitone alterations:

1s = Sharp (raise by semitone) 1b = Flat (lower by semitone)

### Valid Accidentals
- **Sharps**: 1s, 2s, 4s, 5s, 6s
- **Flats**: 2b, 3b, 5b, 6b, 7b

Note: 3s=4, 4b=3, 7s=1`, 1b=7, (enharmonic equivalents)

## Modifier Combination Order

When combining modifiers, use this order:

### Valid Accidentals
- **Sharps**: 1s, 2s, 4s, 5s, 6s
- **Flats**: 2b, 3b, 5b, 6b, 7b

Note: 3s=4, 4b=3, 7s=1`, 1b=7, (enharmonic equivalents)

## Modifier Combination Order

When combining modifiers, use this order:

```ems
[octave_down][note][accidental][duration][octave_up]
```

Examples:

`1s,` = C# down one octave, one beat
`2b-1` = Db half beat, then C up one octave
`4s. ` = F# quarter beat

## Complete Format Structure

```ems
(BPM){default_beat}note[modifiers],note[modifiers],...
```

### Default Values
- **BPM**: 120 if not specified
- **Default Beat**: 4 (quarter note) if not specified
- **Duration**: One beat if not specified
- **Octave**: Middle octave (4th octave) if not specified

## Examples

### Basic Scale

```ems
(120){4}1,2,3,4,5,6,7,1`
```

*C major scale, 120 BPM, quarter notes*

### With Accidentals

```ems
(140){8}1,1s,2,2s,3,4,4s,5,5s,6,6s,7,1`
```

*Chromatic scale, 140 BPM, eighth notes*

### Mixed Durations

```ems
(100){4}1,2-3.4,5_6,7,1`
```

*Mixed rhythm: quarter, half, quarter-quarter, quarter, half-quarter, quarter, quarter*

### Octave Changes

```ems
(120){4}1,2,3,4,5,6,7,1,2,3
```

*Scale spanning from low octave to high octave*

### With Rests

```ems
(120){4}1,0,3,0,5,0,1`
```

*Notes with rests between them*

## Implementation Notes

### Parsing Guidelines
1. Parse BPM first (if present)
2. Parse default beat (if present)
3. Iterate through note sequence
4. For each note, parse in order: octave_down, note, accidental, duration, octave_up
5. Calculate final frequency and duration
6. Generate note structure

### Memory Considerations
- Pre-calculate note arrays at compile time when possible
- Use lookup tables for frequency calculations
- Consider using integer arithmetic for embedded systems

### Error Handling
- Invalid note numbers (not 0-7): treat as rest
- Invalid modifiers: ignore gracefully
- Missing BPM/beat: use defaults
- Empty string: generate silence

## File Extension
Recommended file extension: `.ems`

Example: `twinkle_star.ems`, `happy_birthday.ems`

---

*EMS Format Specification v1.0*  
*Author: MoonFeather & WeaveStar*  
*Date: 2025-07-28*