# Simple Audio Player

A modern desktop audio player built with **C++** and the **JUCE Framework**.

This project was developed as a team project by 3 members. The application supports dual audio players, waveform visualization, playlists, markers, A-B looping, shuffle mode, speed control, and session saving.

## Features

- Two independent audio players
- Play, pause, stop, restart, start, and end controls
- Forward and backward seeking by 10 seconds
- Playlist support with single-click and double-click playback
- Waveform visualization using `AudioThumbnail`
- Playback progress line synced with the waveform
- Add and delete markers to jump to specific timestamps
- A-B looping to repeat a selected part of a track
- Shuffle mode for random playback without immediately repeating the same track
- Speed control from 0.5x to 2.0x
- Volume and mute controls
- Session saving for last file, position, and loop state
- Dark-themed user interface with separate playlist and marker sections

## Tech Stack

- C++
- JUCE Framework
- Object-Oriented Programming
- GUI Development
- Audio Processing
- Event-Driven Programming

## Project Structure

```text
SimpleAudioPlayer/
├── Source/
│   ├── Main.cpp
│   ├── MainComponent.h
│   ├── MainComponent.cpp
│   ├── PlayerGUI.h
│   ├── PlayerGUI.cpp
│   ├── PlayerAudio.h
│   ├── PlayerAudio.cpp
│   ├── WaveformDisplay.h
│   └── WaveformDisplay.cpp
├── SimpleAudioPlayer.jucer
├── README.md
├── LICENSE
└── .gitignore
