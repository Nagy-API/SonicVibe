#pragma once							// PlayerAudio.h
#include <JuceHeader.h>
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
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    bool isLooping = false; 
    double loopStart = 0.0; 
    double loopEnd = 0.0; 
    bool isABLooping = false; 



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};
//PlayerAudio.h
