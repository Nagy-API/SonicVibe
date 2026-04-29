#include "MainComponent.h"

MainComponent::MainComponent()
{
    addAndMakeVisible(player1);
    addAndMakeVisible(player2);

    player1.setName("Player 1");
    player2.setName("Player 2");

    mixer.addInputSource(&player1.getAudioSource(), false);
    mixer.addInputSource(&player2.getAudioSource(), false);

    setSize(1350, 800);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    mixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
}
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixer.getNextAudioBlock(bufferToFill);
}
void MainComponent::releaseResources()
{
    mixer.releaseResources();
}
void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    int halfHeight = area.getHeight() / 2;
    player1.setBounds(area.removeFromTop(halfHeight).reduced(5));
    player2.setBounds(area.reduced(5));
}