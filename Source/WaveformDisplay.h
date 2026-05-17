#pragma once
#include <JuceHeader.h>

class WaveformDisplay : public juce::Component,
    public juce::ChangeListener
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void loadFile(const juce::File& audioFile);
    void setPosition(double relativePosition);

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    double position;
    bool fileLoaded;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};