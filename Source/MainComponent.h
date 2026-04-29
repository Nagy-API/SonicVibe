#pragma once					
#include <JuceHeader.h>
#include "PlayerGUI.h"
using namespace juce;
using namespace std;
class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;

private:
    PlayerGUI player1;
    PlayerGUI player2;
    juce::MixerAudioSource mixer;//jana

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent);
};