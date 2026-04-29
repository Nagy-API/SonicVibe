#include <JuceHeader.h>
#include "PlayerAudio.h"

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
}

PlayerAudio::~PlayerAudio()
{
    transportSource.setSource(nullptr);
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
    checkAndLoopSegment();
}

void PlayerAudio::releaseResources()
{
    transportSource.releaseResources();
}

bool PlayerAudio::loadFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    if (auto* reader = formatManager.createReaderFor(file))
    {
        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();

        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

        transportSource.setSource(
            readerSource.get(),
            0,
            nullptr,
            reader->sampleRate
        );

        return true;
    }

    return false;
}

void PlayerAudio::start()
{
    transportSource.start();
}

void PlayerAudio::stop()
{
    transportSource.stop();
}

void PlayerAudio::setGain(float gain)
{
    transportSource.setGain(gain);
}

void PlayerAudio::setPosition(double pos)
{
    transportSource.setPosition(pos);
}

double PlayerAudio::getPosition() const
{
    return transportSource.getCurrentPosition();
}

double PlayerAudio::getLength() const
{
    return transportSource.getLengthInSeconds();
}

void PlayerAudio::setLooping(bool shouldLoop)
{
    isLooping = shouldLoop;

    if (readerSource != nullptr)
        readerSource->setLooping(isLooping);
}

juce::String PlayerAudio::getFormattedLength() const
{
    double lengthInSeconds = transportSource.getLengthInSeconds();

    int minutes = static_cast<int>(lengthInSeconds) / 60;
    int seconds = static_cast<int>(lengthInSeconds) % 60;

    return juce::String::formatted("%02d:%02d", minutes, seconds);
}

void PlayerAudio::setLoopPoints(double start, double end)
{
    loopStart = juce::jmax(0.0, start);
    loopEnd = juce::jmax(loopStart, end);
}

void PlayerAudio::enableABLoop(bool enable)
{
    isABLooping = enable;
}

void PlayerAudio::checkAndLoopSegment()
{
    if (isABLooping && loopEnd > loopStart)
    {
        double currentPos = transportSource.getCurrentPosition();

        if (currentPos >= loopEnd)
        {
            transportSource.setPosition(loopStart);

            if (transportSource.isPlaying())
                transportSource.start();
        }
    }
}

void PlayerAudio::setSpeed(double ratio)
{
    if (readerSource == nullptr)
        return;

    double currentPosition = transportSource.getCurrentPosition();
    bool wasPlaying = transportSource.isPlaying();

    transportSource.stop();

    double originalSampleRate = readerSource->getAudioFormatReader()->sampleRate;

    transportSource.setSource(
        readerSource.get(),
        0,
        nullptr,
        originalSampleRate * ratio
    );

    transportSource.setPosition(currentPosition);

    if (wasPlaying)
        transportSource.start();
}