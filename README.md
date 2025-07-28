# EMS (Embedded Music Script) Parser

English Version | [中文版本](README_zh.md)

A lightweight music notation format designed for embedded systems and monophonic instruments.

## Features

- Simple string-based notation: `(120){4}1,2,3,4,5,6,7,1^`
- Minimal memory footprint
- Easy parsing for microcontrollers
- Human-readable format
- Extensible syntax

## Format

- See [Format](doc/pages/format.md)

## Example
```c
play_music_string("(120){4}1,2,3,4,5,6,7,1^");
// BPM=120, quarter notes, C major scale
```

## TODO

- [ ] Format & Documentation
- [ ] transform from MIDI
- [ ] parser for C language

## Credit

- The inspiration of EMS is from [simai](https://w.atwiki.jp/simai/pages/1003.html) created by `Celeca`.