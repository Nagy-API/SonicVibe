#pragma once						// PlayerGUI.h
#include <JuceHeader.h>
#include "PlayerAudio.h"


using namespace juce;
using namespace std;

class MarkerListModel : public juce::ListBoxModel//new menna
{
public:
    std::vector<std::pair<int, double>>* markers = nullptr;
    std::function<void(int)> onMarkerClicked;

    int getNumRows() override
    {
        return markers ? (int)markers->size() : 0;
    }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (!markers || row >= (int)markers->size()) return;

        if (rowIsSelected) g.fillAll(juce::Colours::darkgrey);
        else g.fillAll(juce::Colours::black);

        g.setColour(juce::Colours::white);
        auto [num, time] = (*markers)[row];
        int minutes = (int)(time) / 60;
        int seconds = (int)(time) % 60;
        juce::String text = "Marker " + juce::String(num)
            + " (" + juce::String::formatted("%02d:%02d", minutes, seconds) + ")";
        g.drawText(text, 5, 0, width - 10, height, juce::Justification::centredLeft);
    }

    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        if (onMarkerClicked) onMarkerClicked(row);
    }
};


class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::ListBoxModel,
    public juce::Timer//jana
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void paint(juce::Graphics& g) override;
    int getNumRows() override;//nagy
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;//nagy
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;//nagy
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;//nagy
    void selectedRowsChanged(int lastRowSelected) override;//nagy
    void timerCallback() override;//jana
    void sliderDragStarted(juce::Slider* slider) override;//new menna
    void sliderDragEnded(juce::Slider* slider) override;//new menna
    MarkerListModel markerModel;//new menna


    juce::AudioTransportSource& getAudioSource() { return playerAudio.getTransportSource(); }//jana


private:
    PlayerAudio playerAudio;

    // GUI elements
    juce::TextButton playPauseButton{ "Play/Pause" };//nagy
    juce::TextButton startButton{ "Start" };
    juce::TextButton endButton{ "End" };
    juce::TextButton loadButton{ "Load File" };
    juce::TextButton restartButton{ "Restart" };//nagy
    juce::TextButton stopButton{ "Stop" };//nagy
    juce::Label titleLabel{ {}, "Name: -" };//nagy
    juce::Label durationLabel{ {}, "Time: 00:00" };//nagy
    juce::Slider volumeSlider;
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop" };
    juce::TextButton setAButton{ "Set A" };//new menna
    juce::TextButton setBButton{ "Set B" };//new menna
    juce::TextButton abLoopButton{ "A-B Loop" };//new menna
    bool isLooping = false; //menna
    std::vector<juce::File> playlist;//nagy
    double pointA = 0.0;//new menna
    double pointB = 0.0;//new menna
    bool abLoopEnabled = false;//new menna

    juce::ListBox playlistBox{ "List", this };//nagy
    juce::TextButton forwardButton{ " +10s" };//nagy
    juce::TextButton backwardButton{ " -10s" };//nagy
    juce::TextButton shuffleButton{ "Shuffle: OFF" }; //new menna
    bool isShuffle = false;

    juce::Slider positionSlider;//jana
    juce::Label positionLabel{ {}, "00:00 / 00:00" };//jana

    juce::TextButton addMarkerButton{ "Add Marker" };//new menna
    juce::ListBox markerListBox{ "Markers", this };//new menna
    std::vector<std::pair<int, double>> markers; // new menna
    juce::TextButton deleteMarkerButton{ "Delete Marker" };// new menna

    juce::Slider speedSlider;//jana
    juce::Label speedLabel{ {}, "Speed: 1.0x" };//jana


    juce::AudioFormatManager thumbnailFormatManager;//new menna
    juce::AudioThumbnailCache thumbnailCache{ 5 }; //new menna
    juce::AudioThumbnail thumbnail{ 512, thumbnailFormatManager, thumbnailCache };//new menna
    bool thumbnailLoaded = false;//new menna


    std::unique_ptr<juce::FileChooser> fileChooser;

    // Event handlers
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

    void saveSession();//jana
    void loadSession();//jana


    bool isPlaying = false;//nagy

    bool isMuted = false;
    float previousGain = 0.5f;//jana
    juce::Rectangle<int> thumbnailArea; 


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};

//PlayerGUI.h