#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include "PlayerGUI.h"



class IntroOverlay : public juce::Component,
    private juce::Timer
{
public:
    IntroOverlay();
    ~IntroOverlay() override;

    bool startIntro(const juce::File& introFile);
    void showStatusMessage(const juce::String& message, int milliseconds = 3500);
    void stopIntro();

    void resized() override;
    void paint(juce::Graphics& g) override;
    bool keyPressed(const juce::KeyPress& key) override;

    std::function<void()> onFinished;
    juce::TextButton skipButton{ "Skip" };

private:
    void timerCallback() override;
    void finishIntro();

    bool introLoaded = false;
    int statusTicksRemaining = 0;
    int playbackTicks = 0;
    int finishAfterTicks = 0;
    juce::String statusMessage{ "Loading intro..." };

    std::unique_ptr<juce::WebBrowserComponent> browser;
    juce::File htmlFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntroOverlay)
};

class AboutOverlay : public juce::Component
{
public:
    AboutOverlay();

    void setLogoImage(const juce::Image& newLogoImage);
    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::TextButton closeButton{ "Close" };

private:
    juce::Image logoImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutOverlay)
};

class MainComponent : public juce::AudioAppComponent,
    private juce::Slider::Listener,
    private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    enum class ViewMode
    {
        PulseOne,
        PulseTwo,
        DualView
    };

    void setViewMode(ViewMode newMode);
    void updateViewButtons();
    void styleNavButton(juce::TextButton& button);
    void styleMixerSlider(juce::Slider& slider);
    void showAboutOverlay();
    void hideAboutOverlay();
    void startIntroIfAvailable();
    void finishIntroOverlay();
    juce::File findIntroVideoFile() const;
    juce::Image loadAboutLogoImage() const;
    juce::File findAboutLogoFile() const;
    void sliderValueChanged(juce::Slider* slider) override;

    float getPulseOneCrossfadeGain() const;
    float getPulseTwoCrossfadeGain() const;

    PlayerGUI player1;
    PlayerGUI player2;

    juce::TextButton pulseOneButton{ "Pulse One" };
    juce::TextButton pulseTwoButton{ "Pulse Two" };
    juce::TextButton dualViewButton{ "Dual View" };
    juce::TextButton aboutButton{ "About" };

    AboutOverlay aboutOverlay;
    bool aboutOverlayOpen = false;

    IntroOverlay introOverlay;
    bool introOverlayOpen = false;

    juce::Label masterVolumeLabel{ {}, "Master" };
    juce::Slider masterVolumeSlider;

    juce::Label crossfaderLabel{ {}, "Crossfader" };
    juce::Label crossLeftLabel{ {}, "Pulse One" };
    juce::Label crossRightLabel{ {}, "Pulse Two" };
    juce::Slider crossfaderSlider;

    ViewMode currentView = ViewMode::PulseOne;

    // One shared tooltip host prevents duplicate tooltip popups in Dual View.
    juce::TooltipWindow tooltipWindow;

    juce::AudioBuffer<float> pulseOneBuffer;
    juce::AudioBuffer<float> pulseTwoBuffer;

    std::atomic<float> masterGain{ 1.0f };
    std::atomic<float> crossfadeAmount{ 0.5f };

    bool introStartAttempted = false;
    int introStartupChecks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
