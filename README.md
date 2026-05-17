# SonicVibe

SonicVibe is a JUCE-based desktop audio player built for playlist playback, waveform seeking, synced lyrics, cue markers, tempo control, A/B looping, and a neon intro experience.

## Highlights

- Dual Pulse player views: Pulse One, Pulse Two, and Dual View
- Playlist loading, saving, reordering, restoring, and queue management
- Waveform seek display
- Synced lyrics panel
- Cue markers with jump support
- Tempo control powered by SoundTouch
- A/B loop, repeat, shuffle, mute, and navigation controls
- Neon WebView intro video using `SonicVibeAssets/intro.mp4`
- Polished About modal with SonicVibe branding

## Tech Stack

- C++
- JUCE
- Visual Studio 2022
- SoundTouch
- WebView2 / `WebBrowserComponent` for intro playback

## Project Structure

```text
SonicVibe/
├── Source/
│   ├── Main.cpp
│   ├── MainComponent.cpp
│   ├── MainComponent.h
│   ├── PlayerAudio.cpp
│   ├── PlayerAudio.h
│   ├── PlayerGUI.cpp
│   ├── PlayerGUI.h
│   ├── WaveformDisplay.cpp
│   └── WaveformDisplay.h
├── SonicVibeAssets/
│   ├── about_logo.png
│   └── intro.mp4
├── ThirdParty/
│   └── SoundTouch/
└── SonicVibe.jucer
```

## Required JUCE Modules

Make sure these modules are enabled in Projucer:

- `juce_audio_basics`
- `juce_audio_devices`
- `juce_audio_formats`
- `juce_audio_utils`
- `juce_core`
- `juce_data_structures`
- `juce_events`
- `juce_graphics`
- `juce_gui_basics`
- `juce_gui_extra`
- `juce_video`

For the intro video on Windows, `juce_gui_extra` should have WebView2 enabled:

```text
JUCE_WEB_BROWSER = Enabled
JUCE_USE_WIN_WEBVIEW2 = Enabled
JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING = Disabled
```

## Build Instructions

1. Open `SonicVibe.jucer` in Projucer.
2. Confirm the JUCE module paths are correct.
3. Save and open in Visual Studio 2022.
4. Build the `SonicVibe_App` target.
5. Keep `SonicVibeAssets/intro.mp4` and `SonicVibeAssets/about_logo.png` inside the project folder.

## Git LFS

This project uses Git LFS for video/audio assets such as `intro.mp4`.

Before pushing for the first time:

```bash
git lfs install
git lfs track "*.mp4"
git add .gitattributes
```

## Notes

Build outputs, Visual Studio cache files, and generated intro frame fallback assets are intentionally ignored.
