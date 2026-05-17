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
    expectedBlockSize = juce::jmax(64, samplesPerBlockExpected);
    deviceSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    configureTimeStretch(soundTouchChannels);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (std::abs(speedRatio - 1.0) < 0.001)
    {
        transportSource.getNextAudioBlock(bufferToFill);
        checkAndLoopSegment();
        return;
    }

    renderWithSoundTouch(bufferToFill);
    checkAndLoopSegment();
}

void PlayerAudio::releaseResources()
{
    transportSource.releaseResources();
    resetTimeStretch();
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
        resetTimeStretch();

        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
        configureTimeStretch(soundTouchChannels);
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
    resetTimeStretch();
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
    const double lengthInSeconds = transportSource.getLengthInSeconds();
    const int minutes = static_cast<int>(lengthInSeconds) / 60;
    const int seconds = static_cast<int>(lengthInSeconds) % 60;
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
        const double currentPos = transportSource.getCurrentPosition();

        if (currentPos >= loopEnd)
        {
            transportSource.setPosition(loopStart);
            resetTimeStretch();

            if (transportSource.isPlaying())
                transportSource.start();
        }
    }
}

void PlayerAudio::setSpeed(double ratio)
{
    speedRatio = juce::jlimit(0.50, 2.00, ratio);
    soundTouch.setTempo(speedRatio);
    resetTimeStretch();
}

void PlayerAudio::resetTimeStretch()
{
    soundTouch.clear();
}

void PlayerAudio::configureTimeStretch(int numChannels)
{
    soundTouchChannels = juce::jlimit(1, 2, numChannels);
    soundTouch.setSampleRate(static_cast<uint>(deviceSampleRate));
    soundTouch.setChannels(static_cast<uint>(soundTouchChannels));
    soundTouch.setTempo(speedRatio);

    // Lower values reduce latency, which feels better for seeking and lyrics sync.
    soundTouch.setSetting(SETTING_SEQUENCE_MS, 40);
    soundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
    soundTouch.setSetting(SETTING_OVERLAP_MS, 8);

    resetTimeStretch();
}

void PlayerAudio::renderWithSoundTouch(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* output = bufferToFill.buffer;
    if (output == nullptr || bufferToFill.numSamples <= 0)
        return;

    const int outputChannels = output->getNumChannels();
    const int channels = juce::jlimit(1, 2, outputChannels);

    if (channels != soundTouchChannels)
        configureTimeStretch(channels);

    output->clear(bufferToFill.startSample, bufferToFill.numSamples);

    int produced = 0;
    int safetyCounter = 0;

    interleavedOutput.resize(static_cast<size_t>(bufferToFill.numSamples * channels));

    while (produced < bufferToFill.numSamples && safetyCounter++ < 32)
    {
        const uint available = soundTouch.numSamples();

        if (available > 0)
        {
            const uint wanted = static_cast<uint>(bufferToFill.numSamples - produced);
            const uint toReceive = juce::jmin(available, wanted);

            const uint received = soundTouch.receiveSamples(interleavedOutput.data(), toReceive);

            for (int sample = 0; sample < static_cast<int>(received); ++sample)
            {
                for (int ch = 0; ch < channels; ++ch)
                {
                    output->setSample(ch,
                        bufferToFill.startSample + produced + sample,
                        interleavedOutput[static_cast<size_t>(sample * channels + ch)]);
                }
            }

            // If the audio device has more than two output channels, copy stereo/mono sensibly.
            for (int ch = channels; ch < outputChannels; ++ch)
            {
                const int sourceChannel = channels == 1 ? 0 : (ch % channels);
                output->copyFrom(ch,
                    bufferToFill.startSample + produced,
                    *output,
                    sourceChannel,
                    bufferToFill.startSample + produced,
                    static_cast<int>(received));
            }

            produced += static_cast<int>(received);
            continue;
        }

        const int inputSamples = juce::jmax(expectedBlockSize, bufferToFill.numSamples);
        inputBuffer.setSize(channels, inputSamples, false, false, true);
        inputBuffer.clear();

        juce::AudioSourceChannelInfo inputInfo(&inputBuffer, 0, inputSamples);
        transportSource.getNextAudioBlock(inputInfo);
        checkAndLoopSegment();

        interleavedInput.resize(static_cast<size_t>(inputSamples * channels));

        for (int sample = 0; sample < inputSamples; ++sample)
        {
            for (int ch = 0; ch < channels; ++ch)
                interleavedInput[static_cast<size_t>(sample * channels + ch)] = inputBuffer.getSample(ch, sample);
        }

        soundTouch.putSamples(interleavedInput.data(), static_cast<uint>(inputSamples));
    }

    if (produced < bufferToFill.numSamples)
    {
        for (int ch = 0; ch < outputChannels; ++ch)
            output->clear(ch, bufferToFill.startSample + produced, bufferToFill.numSamples - produced);
    }
}
