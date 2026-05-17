#pragma once

#include <JuceHeader.h>
#include "../ThirdParty/SoundTouch/include/SoundTouch.h"

class PlayerAudio
{
public:
    PlayerAudio();
    ~PlayerAudio();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    bool loadFile(const juce::File& file);
    void start();
    void stop();
    void setGain(float gain);
    void setPosition(double pos);
    double getPosition() const;
    double getLength() const;
    void setLooping(bool shouldLoop);
    juce::String getFormattedLength() const;
    void setLoopPoints(double start, double end);
    void enableABLoop(bool enable);
    void checkAndLoopSegment();
    void setSpeed(double ratio);

    juce::AudioTransportSource& getTransportSource() { return transportSource; }

private:
    void resetTimeStretch();
    void configureTimeStretch(int numChannels);
    void renderWithSoundTouch(const juce::AudioSourceChannelInfo& bufferToFill);

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    soundtouch::SoundTouch soundTouch;
    juce::AudioBuffer<float> inputBuffer;
    std::vector<float> interleavedInput;
    std::vector<float> interleavedOutput;

    double deviceSampleRate = 44100.0;
    int expectedBlockSize = 512;
    int soundTouchChannels = 2;
    double speedRatio = 1.0;

    bool isLooping = false;
    double loopStart = 0.0;
    double loopEnd = 0.0;
    bool isABLooping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};
