#pragma once

#include <JuceHeader.h>
#include "PlayerAudio.h"

class MarkerListModel : public juce::ListBoxModel
{
public:
    std::vector<std::pair<int, double>>* markers = nullptr;
    std::function<void(int)> onMarkerClicked;

    int getNumRows() override
    {
        return markers != nullptr ? static_cast<int>(markers->size()) : 0;
    }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (markers == nullptr || row < 0 || row >= static_cast<int>(markers->size()))
            return;

        g.fillAll(juce::Colour::fromRGB(2, 4, 15));

        auto rowArea = juce::Rectangle<float>(5.0f, 3.0f, static_cast<float>(width - 10), static_cast<float>(height - 6));
        if (rowIsSelected)
        {
            juce::ColourGradient rowGrad(juce::Colour::fromRGB(64, 28, 110).withAlpha(0.86f), rowArea.getX(), rowArea.getCentreY(),
                juce::Colour::fromRGB(8, 92, 140).withAlpha(0.80f), rowArea.getRight(), rowArea.getCentreY(), false);
            g.setGradientFill(rowGrad);
            g.fillRoundedRectangle(rowArea, 7.0f);
            g.setColour(juce::Colour::fromRGB(255, 55, 214).withAlpha(0.28f));
            g.drawRoundedRectangle(rowArea, 7.0f, 1.0f);
        }

        const auto [num, time] = (*markers)[static_cast<size_t>(row)];
        const int minutes = static_cast<int>(time) / 60;
        const int seconds = static_cast<int>(time) % 60;

        g.setColour(rowIsSelected ? juce::Colours::white : juce::Colours::white.withAlpha(0.82f));
        g.setFont(juce::Font(juce::FontOptions(12.5f, rowIsSelected ? juce::Font::bold : juce::Font::plain)));
        g.drawText("Marker " + juce::String(num) + " (" + juce::String::formatted("%02d:%02d", minutes, seconds) + ")",
            10, 0, width - 16, height, juce::Justification::centredLeft);
    }

    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        if (onMarkerClicked)
            onMarkerClicked(row);
    }
};


struct LyricLine
{
    double timeSeconds = -1.0;
    juce::String text;
};

class LyricsListModel : public juce::ListBoxModel
{
public:
    std::vector<LyricLine>* lyrics = nullptr;
    int* currentIndex = nullptr;
    std::function<void(int)> onLyricClicked;

    int getNumRows() override
    {
        return lyrics != nullptr ? static_cast<int>(lyrics->size()) : 0;
    }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (lyrics == nullptr || row < 0 || row >= static_cast<int>(lyrics->size()))
            return;

        const int active = currentIndex != nullptr ? *currentIndex : -1;
        const bool isCurrent = row == active;
        const int distance = active >= 0 ? std::abs(row - active) : 4;

        // More Apple/Spotify-like: the current line feels alive, nearby lines softly fade,
        // and selected non-current lines get a quiet highlight.
        const float proximity = active >= 0 ? juce::jlimit(0.0f, 1.0f, 1.0f - static_cast<float>(distance) * 0.22f)
            : 0.0f;

        if (isCurrent)
        {
            auto glow = juce::Rectangle<float>(8.0f, 5.0f, static_cast<float>(width - 16), static_cast<float>(height - 10));
            juce::ColourGradient gradient(juce::Colour::fromRGB(8, 92, 140).withAlpha(0.92f), glow.getX(), glow.getCentreY(),
                juce::Colour::fromRGB(5, 16, 38).withAlpha(0.78f), glow.getRight(), glow.getCentreY(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(glow, 18.0f);

            g.setColour(juce::Colour::fromRGB(38, 231, 255).withAlpha(0.42f));
            g.drawRoundedRectangle(glow, 18.0f, 1.6f);
        }
        else if (rowIsSelected)
        {
            g.setColour(juce::Colour::fromRGB(12, 18, 58).withAlpha(0.68f));
            g.fillRoundedRectangle(juce::Rectangle<float>(10.0f, 8.0f, static_cast<float>(width - 20), static_cast<float>(height - 16)), 12.0f);
        }

        const auto& line = (*lyrics)[static_cast<size_t>(row)];

        const float alpha = isCurrent ? 1.0f : juce::jmax(0.30f, 0.40f + proximity * 0.34f);
        const float fontSize = isCurrent ? 22.0f : (14.5f + proximity * 1.8f);

        g.setColour(juce::Colours::white.withAlpha(alpha));
        g.setFont(juce::Font(juce::FontOptions(fontSize, isCurrent ? juce::Font::bold : juce::Font::plain)));
        g.drawFittedText(line.text, 20, 0, width - 40, height,
            juce::Justification::centred, 2, 0.86f);
    }

    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        if (onLyricClicked)
            onLyricClicked(row);
    }

    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override
    {
        if (onLyricClicked)
            onLyricClicked(row);
    }
};



class RestoreSessionOverlay : public juce::Component
{
public:
    RestoreSessionOverlay();

    void setPulseTitle(const juce::String& newTitle);
    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::TextButton restoreButton{ "Restore Playlist" };
    juce::TextButton startEmptyButton{ "Start Empty" };

private:
    juce::String pulseTitle{ "Pulse" };
};

class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::ListBoxModel,
    public juce::Timer,
    public juce::FileDragAndDropTarget
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    void setPulseTitle(const juce::String& newTitle);
    void setCompactMode(bool shouldBeCompact);
    void promptToRestoreSessionIfAvailable();

    void resized() override;
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    bool keyPressed(const juce::KeyPress& key) override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;
    void selectedRowsChanged(int lastRowSelected) override;
    void deleteKeyPressed(int lastRowSelected) override;
    void returnKeyPressed(int lastRowSelected) override;

    void timerCallback() override;
    void sliderDragStarted(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;

    juce::AudioTransportSource& getAudioSource() { return playerAudio.getTransportSource(); }

private:
    PlayerAudio playerAudio;
    juce::String pulseTitle{ "Pulse" };
    bool compactMode = false;
    enum class SidebarTab { Playlist, Lyrics, Markers, TrackInfo };
    SidebarTab activeSidebarTab = SidebarTab::Playlist;
    bool playlistPanelOpen = false; // legacy flag kept for session-safe compatibility
    bool markerPanelOpen = false;   // legacy flag kept for session-safe compatibility
    bool sessionLoaded = false;
    bool restorePromptVisible = false;

    RestoreSessionOverlay restoreOverlay;

    juce::TextButton playlistToggleButton{ "Playlist" };
    juce::TextButton lyricsToggleButton{ "Lyrics" };
    juce::TextButton markerToggleButton{ "Markers" };
    juce::TextButton trackInfoToggleButton{ "Info" };

    juce::TextButton loadButton{ "Load" };
    juce::TextButton loadQueueButton{ "Load" };
    juce::TextButton saveQueueButton{ "Save" };
    juce::TextButton removeTrackButton{ "Remove" };
    juce::TextButton clearQueueButton{ "Clear" };
    juce::TextButton moveUpButton{ "Move Up" };
    juce::TextButton moveDownButton{ "Move Down" };

    juce::TextButton autoLyricsButton{ "Auto Lyrics On" };
    juce::TextButton loadLyricsButton{ "Load Lyrics" };
    juce::TextButton clearLyricsButton{ "Clear Lyrics" };

    juce::TextButton playPauseButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton previousTrackButton{ "Prev" };
    juce::TextButton nextTrackButton{ "Next" };
    juce::TextButton startButton{ "Start" };
    juce::TextButton endButton{ "End" };
    juce::TextButton backwardButton{ "-10s" };
    juce::TextButton forwardButton{ "+10s" };

    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop Off" };
    juce::TextButton shuffleButton{ "Shuffle Off" };
    juce::TextButton repeatOneButton{ "Repeat Off" };

    juce::TextButton setAButton{ "Set A" };
    juce::TextButton setBButton{ "Set B" };
    juce::TextButton abLoopButton{ "A-B Off" };
    juce::TextButton addMarkerButton{ "Add Marker" };
    juce::TextButton deleteMarkerButton{ "Delete Marker" };

    juce::Label titleLabel{ {}, "Name: -" };
    juce::Label durationLabel{ {}, "Length: 00:00" };
    juce::Label positionLabel{ {}, "00:00 / 00:00" };
    juce::Label speedLabel{ {}, "Tempo: 1.00x" };
    juce::Label queueStatusLabel{ {}, "Playlist: 0 tracks" };
    juce::Label lyricsStatusLabel{ {}, "Lyrics: none" };

    juce::Slider volumeSlider;
    juce::Slider positionSlider;
    juce::Slider speedSlider;

    std::vector<juce::File> playlist;
    juce::ListBox playlistBox{ "Playlist", this };

    MarkerListModel markerModel;
    juce::ListBox markerListBox{ "Cue Markers" };
    std::vector<std::pair<int, double>> markers;

    LyricsListModel lyricsModel;
    juce::ListBox lyricsListBox{ "Lyrics" };
    std::vector<LyricLine> lyrics;
    int currentLyricIndex = -1;
    int lyricsTargetScrollY = 0;
    double lyricsSmoothScrollY = 0.0;
    double lyricsScrollVelocityY = 0.0;
    bool lyricsScrollInitialised = false;
    bool lyricsScrollAnimating = false;

    double speedRatio = 1.0;
    double playbackAnchorPosition = 0.0;
    double playbackAnchorTimeMs = 0.0;
    juce::File currentLyricsFile;
    juce::File currentAudioFile;
    bool autoLyricsEnabled = true;

    bool isPlaying = false;
    bool isMuted = false;
    bool isLooping = false;
    bool isShuffle = false;
    enum class RepeatMode { Off, One, All };
    RepeatMode repeatMode = RepeatMode::Off;
    bool abLoopEnabled = false;

    float previousGain = 0.5f;
    double pointA = 0.0;
    double pointB = 0.0;

    juce::AudioFormatManager thumbnailFormatManager;
    juce::AudioThumbnailCache thumbnailCache{ 5 };
    juce::AudioThumbnail thumbnail{ 512, thumbnailFormatManager, thumbnailCache };
    bool thumbnailLoaded = false;
    juce::Rectangle<int> thumbnailArea;
    juce::Rectangle<int> overviewArea;
    juce::Rectangle<int> albumArtArea;
    juce::Image albumArtImage;
    double discRotationRadians = 0.0;
    juce::Rectangle<int> transportDeckArea;
    juce::Rectangle<int> markersCardArea;
    juce::Rectangle<int> toolsCardArea;
    juce::Rectangle<int> trackInfoArea;
    juce::Rectangle<int> sidebarPanelArea;

    std::unique_ptr<juce::FileChooser> fileChooser;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

    bool isSupportedAudioFile(const juce::File& file) const;
    bool isKnownUnsupportedAudioFile(const juce::File& file) const;
    void showUnsupportedAudioFormatMessage(const juce::StringArray& fileNames) const;
    bool isSupportedLyricsFile(const juce::File& file) const;
    bool appendAudioFile(const juce::File& file);
    void appendAudioFiles(const juce::Array<juce::File>& files);

    void loadTrackAt(int row, bool shouldStartPlayback);
    void loadRelativeTrack(int offset, bool shouldStartPlayback);
    void moveTrack(int fromRow, int direction);
    void updateQueueStatusLabel();
    void applyTempoFromSlider();
    void updateLyricsStatusLabel();
    void updateCurrentLyric();
    void setCurrentLyricIndex(int newIndex);
    void centerLyricsOnCurrentLine();
    int calculateCenteredLyricsScrollY() const;
    void updateLyricsScrollAnimation();
    void seekToLyricLine(int row);
    double getEffectivePlaybackPosition() const;
    void setSyncedPlaybackPosition(double seconds);
    void markPlaybackStarted();
    void markPlaybackPaused();
    void autoFitLyricsToCurrentTrack(std::vector<LyricLine>& parsedLines, bool isPlainText);
    bool loadLyricsFromFile(const juce::File& file);
    void clearLyrics(bool resetFile = true);
    juce::File findMatchingLyricsFileForTrack(const juce::File& audioFile) const;
    void tryAutoLoadLyricsForTrack(const juce::File& audioFile);
    void removeTrackAt(int row);
    void clearQueue();
    void resetLoadedTrackUI();
    void resetLoopState();
    void cycleRepeatMode();
    void updateRepeatButtonVisuals();
    juce::String getRepeatModeText() const;
    bool isRepeatActive() const;
    void refreshButtonVisibility();
    void updateRestorePromptVisibility();
    void handleTrackFinished();
    void seekFromWaveformX(float mouseX);
    void loadAlbumArtForTrack(const juce::File& audioFile);
    void clearAlbumArt();

    void saveQueueToFile();
    void loadQueueFromFile();

    juce::File getSessionFile() const;
    bool hasRestorableSession() const;
    void saveSession();
    void loadSession();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
