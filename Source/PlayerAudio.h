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
    void setLooping(bool shouldLoop); //mennna
    juce::String getFormattedLength() const;//nagy
    void setLoopPoints(double start, double end); //new menna
    void enableABLoop(bool enable); //new menna
    void checkAndLoopSegment(); //new menna
    void setSpeed(double ratio);//jana

    juce::AudioTransportSource& getTransportSource() { return transportSource; }//jana



private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    bool isLooping = false; //menna
    double loopStart = 0.0; // new menna
    double loopEnd = 0.0; // new menna
    bool isABLooping = false; // new menna



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};
//PlayerAudio.h