#include "MainComponent.h"


IntroOverlay::IntroOverlay()
{
    setInterceptsMouseClicks(true, true);
    setWantsKeyboardFocus(true);

#if JUCE_WINDOWS && (JUCE_USE_WIN_WEBVIEW2 || JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING)
    auto userDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("SonicVibe")
        .getChildFile("WebView2");
    userDataFolder.createDirectory();

    auto winOptions = juce::WebBrowserComponent::Options::WinWebView2{}
        .withUserDataFolder(userDataFolder)
        .withStatusBarDisabled()
        .withBackgroundColour(juce::Colour::fromRGB(1, 2, 10));

    auto options = juce::WebBrowserComponent::Options{}
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        .withWinWebView2Options(winOptions);

    browser = std::make_unique<juce::WebBrowserComponent>(options);
#else
    browser = std::make_unique<juce::WebBrowserComponent>();
#endif

    browser->setVisible(false);
    addAndMakeVisible(*browser);

    skipButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(14, 18, 48).withAlpha(0.92f));
    skipButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(255, 49, 206));
    skipButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.92f));
    skipButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    skipButton.onClick = [this] { finishIntro(); };
    addAndMakeVisible(skipButton);
    skipButton.toFront(false);
}

IntroOverlay::~IntroOverlay()
{
    stopIntro();
}

bool IntroOverlay::startIntro(const juce::File& introFile)
{
    stopIntro();

    if (!introFile.existsAsFile())
    {
        statusMessage = "Intro video not found";
        introLoaded = false;
        return false;
    }

    htmlFile = introFile.getSiblingFile("sonicvibe_intro_webview.html");
    const auto videoUrl = juce::URL(introFile).toString(true);

#if JUCE_WINDOWS && !(JUCE_USE_WIN_WEBVIEW2 || JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING)
    statusMessage = "WebView2 is not enabled in juce_gui_extra";
    introLoaded = false;
    return false;
#endif

    const auto html = juce::String(R"HTML(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
    html, body {
        margin: 0;
        padding: 0;
        width: 100%;
        height: 100%;
        overflow: hidden;
        background: #01020a;
    }
    body::before {
        content: "";
        position: fixed;
        inset: 0;
        background:
            radial-gradient(circle at 28% 38%, rgba(31,229,255,0.16), transparent 44%),
            radial-gradient(circle at 76% 42%, rgba(255,49,206,0.15), transparent 48%);
        pointer-events: none;
        z-index: 1;
    }
    video {
        position: fixed;
        inset: 0;
        width: 100vw;
        height: 100vh;
        object-fit: cover;
        background: #01020a;
    }
</style>
</head>
<body>
    <video autoplay muted playsinline preload="auto">
        <source src="VIDEO_FILE_URL" type="video/mp4">
    </video>
</body>
</html>
)HTML").replace("VIDEO_FILE_URL", videoUrl);

    if (!htmlFile.replaceWithText(html))
    {
        statusMessage = "Could not create intro web page";
        introLoaded = false;
        return false;
    }

    introLoaded = true;
    statusMessage.clear();
    playbackTicks = 0;

    // The current SonicVibe intro is 8 seconds. Add a small buffer to avoid cutting the last frame.
    finishAfterTicks = 9 * 20;

    browser->setVisible(true);
    browser->goToURL(juce::URL(htmlFile).toString(true));

    setVisible(true);
    toFront(false);
    skipButton.toFront(false);
    grabKeyboardFocus();
    startTimerHz(20);
    repaint();
    return true;
}

void IntroOverlay::showStatusMessage(const juce::String& message, int milliseconds)
{
    stopIntro();
    introLoaded = false;
    statusMessage = message;
    statusTicksRemaining = juce::jmax(1, milliseconds / 50);
    setVisible(true);
    toFront(false);
    grabKeyboardFocus();
    startTimerHz(20);
    repaint();
}

void IntroOverlay::stopIntro()
{
    stopTimer();
    statusTicksRemaining = 0;
    playbackTicks = 0;
    finishAfterTicks = 0;

    if (browser != nullptr) browser->stop();
    if (browser != nullptr) browser->goToURL("about:blank");
    if (browser != nullptr) browser->setVisible(false);

    introLoaded = false;
}

void IntroOverlay::resized()
{
    if (browser != nullptr) browser->setBounds(getLocalBounds());

    auto skipArea = getLocalBounds().reduced(28);
    skipButton.setBounds(skipArea.removeFromBottom(40).removeFromRight(96));
    skipButton.toFront(false);
}

void IntroOverlay::paint(juce::Graphics& g)
{
    const auto accent = juce::Colour::fromRGB(31, 229, 255);
    const auto accentPink = juce::Colour::fromRGB(255, 49, 206);
    const auto deep = juce::Colour::fromRGB(1, 2, 10);

    g.fillAll(deep);

    if (!introLoaded)
    {
        juce::ColourGradient cyanGlow(accent.withAlpha(0.22f),
            static_cast<float>(getWidth()) * 0.28f,
            static_cast<float>(getHeight()) * 0.38f,
            juce::Colours::transparentBlack,
            static_cast<float>(getWidth()) * 0.80f,
            static_cast<float>(getHeight()) * 0.82f,
            true);
        g.setGradientFill(cyanGlow);
        g.fillRect(getLocalBounds());

        juce::ColourGradient magentaGlow(accentPink.withAlpha(0.18f),
            static_cast<float>(getWidth()) * 0.76f,
            static_cast<float>(getHeight()) * 0.42f,
            juce::Colours::transparentBlack,
            static_cast<float>(getWidth()) * 0.30f,
            static_cast<float>(getHeight()) * 0.88f,
            true);
        g.setGradientFill(magentaGlow);
        g.fillRect(getLocalBounds());
    }

    if (!introLoaded && statusMessage.isNotEmpty())
    {
        g.setColour(juce::Colours::white.withAlpha(0.92f));
        g.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold)));
        g.drawText(statusMessage, getLocalBounds().reduced(40), juce::Justification::centred);
    }

    g.setColour(accent.withAlpha(0.28f));
    g.drawRoundedRectangle(getLocalBounds().reduced(10).toFloat(), 18.0f, 1.4f);
}

bool IntroOverlay::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey
        || key == juce::KeyPress::spaceKey
        || key == juce::KeyPress::returnKey)
    {
        finishIntro();
        return true;
    }

    return false;
}

void IntroOverlay::timerCallback()
{
    if (!introLoaded)
    {
        if (statusTicksRemaining > 0 && --statusTicksRemaining == 0)
            finishIntro();

        return;
    }

    if (++playbackTicks >= finishAfterTicks)
        finishIntro();
}

void IntroOverlay::finishIntro()
{
    if (onFinished)
        onFinished();
}



AboutOverlay::AboutOverlay()
{
    setInterceptsMouseClicks(true, true);
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(224, 82, 246));
    closeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(31, 229, 255));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour::fromRGB(8, 9, 24));
    closeButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    addAndMakeVisible(closeButton);
}

void AboutOverlay::setLogoImage(const juce::Image& newLogoImage)
{
    logoImage = newLogoImage;
    repaint();
}

void AboutOverlay::resized()
{
    // Pixora-style modal: portrait artwork on the left, details on the right.
    auto card = getLocalBounds().withSizeKeepingCentre(800, 520);
    auto content = card.reduced(30, 28);
    closeButton.setBounds(content.withTrimmedTop(content.getHeight() - 44).reduced(2, 0));
}

void AboutOverlay::paint(juce::Graphics& g)
{
    const auto accent = juce::Colour::fromRGB(31, 229, 255);
    const auto accentPink = juce::Colour::fromRGB(255, 49, 206);
    const auto violet = juce::Colour::fromRGB(112, 58, 255);

    g.fillAll(juce::Colours::black.withAlpha(0.64f));
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    auto card = getLocalBounds().withSizeKeepingCentre(800, 520).toFloat();

    g.setColour(juce::Colours::black.withAlpha(0.46f));
    g.fillRoundedRectangle(card.translated(0.0f, 8.0f), 22.0f);

    juce::ColourGradient cardGradient(juce::Colour::fromRGB(7, 8, 34), card.getX(), card.getY(),
        juce::Colour::fromRGB(8, 8, 30), card.getRight(), card.getBottom(), false);
    g.setGradientFill(cardGradient);
    g.fillRoundedRectangle(card, 22.0f);

    juce::ColourGradient glow(accentPink.withAlpha(0.055f), card.getRight() - 80.0f, card.getY() + 58.0f,
        violet.withAlpha(0.055f), card.getX() + 80.0f, card.getBottom() - 90.0f, false);
    g.setGradientFill(glow);
    g.fillRoundedRectangle(card.reduced(8.0f), 18.0f);

    g.setColour(accent.withAlpha(0.78f));
    g.drawRoundedRectangle(card, 22.0f, 1.4f);
    g.setColour(accentPink.withAlpha(0.38f));
    g.drawRoundedRectangle(card.reduced(3.0f), 19.0f, 0.9f);

    auto area = card.toNearestInt().reduced(28, 26);
    auto top = area.removeFromTop(390);
    auto imagePanel = top.removeFromLeft(285);
    top.removeFromLeft(32);
    auto textPanel = top;

    // Portrait poster frame, like the Pixora About design.
    auto imageBounds = imagePanel.reduced(14, 8);
    auto imageFrame = imageBounds.expanded(12).toFloat();

    g.setColour(juce::Colour::fromRGB(7, 9, 32).withAlpha(0.92f));
    g.fillRoundedRectangle(imageFrame, 14.0f);
    g.setColour(violet.withAlpha(0.42f));
    g.drawRoundedRectangle(imageFrame, 14.0f, 1.0f);

    if (logoImage.isValid())
    {
        juce::Graphics::ScopedSaveState saveState(g);
        g.reduceClipRegion(imageBounds);
        g.drawImageWithin(logoImage,
            imageBounds.getX(), imageBounds.getY(), imageBounds.getWidth(), imageBounds.getHeight(),
            juce::RectanglePlacement::fillDestination | juce::RectanglePlacement::centred);

    }
    else
    {
        auto disc = imageBounds.reduced(42).toFloat();
        g.setColour(juce::Colour::fromRGB(5, 7, 22));
        g.fillEllipse(disc);
        g.setColour(accent.withAlpha(0.85f));
        g.drawEllipse(disc, 2.0f);
        g.setColour(accentPink.withAlpha(0.42f));
        g.drawEllipse(disc.reduced(10.0f), 1.2f);
        g.setColour(juce::Colours::white.withAlpha(0.94f));
        g.setFont(juce::Font(juce::FontOptions(42.0f, juce::Font::bold)));
        g.drawText("SV", disc.toNearestInt(), juce::Justification::centred);
    }

    g.setColour(accent.withAlpha(0.18f));
    g.drawRoundedRectangle(imageBounds.toFloat(), 10.0f, 0.9f);

    g.setColour(accentPink);
    g.setFont(juce::Font(juce::FontOptions(45.0f, juce::Font::bold)));
    g.drawText("SonicVibe", textPanel.removeFromTop(58), juce::Justification::centredLeft);

    g.setColour(juce::Colours::white.withAlpha(0.90f));
    g.setFont(juce::Font(juce::FontOptions(17.0f, juce::Font::bold)));
    g.drawText("Ride the Vibe", textPanel.removeFromTop(28), juce::Justification::centredLeft);

    g.setColour(accentPink.withAlpha(0.78f));
    g.drawLine(static_cast<float>(textPanel.getX()), static_cast<float>(textPanel.getY() + 6),
        static_cast<float>(textPanel.getRight()), static_cast<float>(textPanel.getY() + 6), 1.2f);
    textPanel.removeFromTop(20);

    g.setColour(juce::Colours::white.withAlpha(0.94f));
    g.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    g.drawText("Version 1.0.0", textPanel.removeFromTop(25), juce::Justification::centredLeft);

    g.setColour(juce::Colours::white.withAlpha(0.72f));
    g.setFont(juce::Font(juce::FontOptions(13.5f)));
    g.drawText("Build 2026", textPanel.removeFromTop(22), juce::Justification::centredLeft);

    textPanel.removeFromTop(8);
    g.setColour(juce::Colours::white.withAlpha(0.80f));
    g.setFont(juce::Font(juce::FontOptions(14.2f)));
    g.drawFittedText("SonicVibe is a dual-pulse audio player built for playlists, synced lyrics, cue markers, waveform seeking, tempo control, and live mixing.",
        textPanel.removeFromTop(96), juce::Justification::topLeft, 4, 0.92f);

    textPanel.removeFromTop(10);
    g.setColour(accent.withAlpha(0.90f));
    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.drawText("Created by SonicVibe's Team", textPanel.removeFromTop(26), juce::Justification::centredLeft);
}

MainComponent::MainComponent()
    : tooltipWindow(this, 650)
{
    player1.setPulseTitle("Pulse One");
    player2.setPulseTitle("Pulse Two");

    for (auto* button : { &pulseOneButton, &pulseTwoButton, &dualViewButton, &aboutButton })
    {
        styleNavButton(*button);
        addAndMakeVisible(button);
    }

    pulseOneButton.onClick = [this] { setViewMode(ViewMode::PulseOne); };
    pulseTwoButton.onClick = [this] { setViewMode(ViewMode::PulseTwo); };
    dualViewButton.onClick = [this] { setViewMode(ViewMode::DualView); };
    aboutButton.onClick = [this] { showAboutOverlay(); };

    for (auto* label : { &masterVolumeLabel, &crossfaderLabel, &crossLeftLabel, &crossRightLabel })
    {
        label->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.78f));
        label->setFont(juce::Font(juce::FontOptions(12.5f, juce::Font::bold)));
        addAndMakeVisible(label);
    }

    masterVolumeSlider.setRange(0.0, 1.0, 0.01);
    masterVolumeSlider.setValue(1.0, juce::dontSendNotification);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 52, 22);
    styleMixerSlider(masterVolumeSlider);
    masterVolumeSlider.addListener(this);
    addAndMakeVisible(masterVolumeSlider);

    crossfaderSlider.setRange(0.0, 1.0, 0.01);
    crossfaderSlider.setValue(0.5, juce::dontSendNotification);
    crossfaderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    styleMixerSlider(crossfaderSlider);
    crossfaderSlider.addListener(this);
    addAndMakeVisible(crossfaderSlider);

    addAndMakeVisible(player1);
    addAndMakeVisible(player2);

    aboutOverlay.closeButton.onClick = [this] { hideAboutOverlay(); };
    aboutOverlayOpen = false;
    aboutOverlay.setVisible(false);
    aboutOverlay.setAlwaysOnTop(true);
    addAndMakeVisible(aboutOverlay);

    introOverlay.onFinished = [this] { finishIntroOverlay(); };
    introOverlayOpen = false;
    introOverlay.setVisible(false);
    introOverlay.setAlwaysOnTop(true);
    addAndMakeVisible(introOverlay);

    setSize(1500, 900);
    setAudioChannels(0, 2);
    setViewMode(ViewMode::PulseOne);

    // Keep About closed on startup. It opens only from the About button.
    aboutOverlayOpen = false;
    aboutOverlay.setVisible(false);

    // Delay the intro until the main native window exists.
    // This keeps the WebBrowser intro stable during startup.
    startTimer(60);
}

MainComponent::~MainComponent()
{
    stopTimer();
    introOverlay.stopIntro();
    masterVolumeSlider.removeListener(this);
    crossfaderSlider.removeListener(this);
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer == nullptr)
        return;

    auto* output = bufferToFill.buffer;
    const int startSample = bufferToFill.startSample;
    const int numSamples = bufferToFill.numSamples;
    const int numChannels = output->getNumChannels();

    output->clear(startSample, numSamples);

    pulseOneBuffer.setSize(numChannels, numSamples, false, false, true);
    pulseTwoBuffer.setSize(numChannels, numSamples, false, false, true);
    pulseOneBuffer.clear();
    pulseTwoBuffer.clear();

    juce::AudioSourceChannelInfo pulseOneInfo(&pulseOneBuffer, 0, numSamples);
    juce::AudioSourceChannelInfo pulseTwoInfo(&pulseTwoBuffer, 0, numSamples);

    player1.getNextAudioBlock(pulseOneInfo);
    player2.getNextAudioBlock(pulseTwoInfo);

    const float master = masterGain.load();
    const float pulseOneGain = getPulseOneCrossfadeGain() * master;
    const float pulseTwoGain = getPulseTwoCrossfadeGain() * master;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        output->addFrom(channel, startSample, pulseOneBuffer, channel, 0, numSamples, pulseOneGain);
        output->addFrom(channel, startSample, pulseTwoBuffer, channel, 0, numSamples, pulseTwoGain);
    }
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    pulseOneBuffer.setSize(0, 0);
    pulseTwoBuffer.setSize(0, 0);
}

void MainComponent::paint(juce::Graphics& g)
{
    const auto deepTop = juce::Colour::fromRGB(2, 4, 18);
    const auto deepBottom = juce::Colour::fromRGB(0, 1, 8);
    const auto panel = juce::Colour::fromRGB(8, 11, 36);
    const auto panel2 = juce::Colour::fromRGB(2, 4, 16);
    const auto accent = juce::Colour::fromRGB(38, 231, 255);
    const auto accentPink = juce::Colour::fromRGB(255, 55, 214);
    const auto accentBlue = juce::Colour::fromRGB(42, 126, 255);
    const auto accentDeep = juce::Colour::fromRGB(136, 74, 255);
    const auto border = juce::Colour::fromRGB(78, 75, 210);

    juce::ColourGradient bg(deepTop, 0.0f, 0.0f, deepBottom, 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bg);
    g.fillAll();

    auto header = getLocalBounds().removeFromTop(116);

    juce::ColourGradient headerGradient(juce::Colour::fromRGB(4, 6, 24), 0.0f, 0.0f,
        juce::Colour::fromRGB(0, 1, 10), 0.0f, static_cast<float>(header.getBottom()), false);
    g.setGradientFill(headerGradient);
    g.fillRect(header);

    auto glow = header.toFloat().reduced(static_cast<float>(getWidth()) * 0.16f, 14.0f);
    juce::ColourGradient glowGradient(accentBlue.withAlpha(0.26f), glow.getCentreX(), glow.getCentreY(),
        juce::Colours::transparentBlack, glow.getRight(), glow.getCentreY(), true);
    g.setGradientFill(glowGradient);
    g.fillRoundedRectangle(glow, 34.0f);

    auto sideGlow = header.toFloat().withTrimmedLeft(static_cast<float>(getWidth()) * 0.52f).reduced(18.0f, 18.0f);
    juce::ColourGradient pinkGlow(accentPink.withAlpha(0.24f), sideGlow.getCentreX(), sideGlow.getCentreY(),
        juce::Colours::transparentBlack, sideGlow.getRight(), sideGlow.getCentreY(), true);
    g.setGradientFill(pinkGlow);
    g.fillRoundedRectangle(sideGlow, 34.0f);

    auto brandArea = header.reduced(22, 0);
    g.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    auto logoLine = brandArea.withHeight(52);

    // Keep the wordmark as one clean draw call. Splitting Sonic/Vibe caused overlap
    // on some font metrics and produced the broken "SonVibe" look.
    g.setColour(accent);
    g.drawText("SonicVibe", logoLine, juce::Justification::centredLeft);

    g.setColour(accentPink.withAlpha(0.88f));
    g.setFont(juce::Font(juce::FontOptions(13.5f, juce::Font::bold)));
    g.drawText("Ride the Vibe", brandArea.withY(62).withHeight(20), juce::Justification::centredLeft);

    auto navShell = juce::Rectangle<int>(getWidth() / 2 - 220, 34, 440, 56);
    g.setColour(juce::Colours::black.withAlpha(0.20f));
    g.fillRoundedRectangle(navShell.toFloat().translated(0.0f, 2.0f), 12.0f);
    g.setColour(panel.withAlpha(0.88f));
    g.fillRoundedRectangle(navShell.toFloat(), 12.0f);
    g.setColour(border.withAlpha(0.78f));
    g.drawRoundedRectangle(navShell.toFloat(), 12.0f, 1.15f);

    auto mixerShell = juce::Rectangle<int>(juce::jmax(getWidth() - 380, getWidth() / 2 + 250), 22, 350, 76);
    g.setColour(juce::Colours::black.withAlpha(0.22f));
    g.fillRoundedRectangle(mixerShell.toFloat().translated(0.0f, 2.0f), 12.0f);
    juce::ColourGradient mixerGrad(panel,
        static_cast<float>(mixerShell.getX()), static_cast<float>(mixerShell.getY()),
        panel2,
        static_cast<float>(mixerShell.getRight()), static_cast<float>(mixerShell.getBottom()),
        false);
    g.setGradientFill(mixerGrad);
    g.fillRoundedRectangle(mixerShell.toFloat(), 12.0f);
    g.setColour(border.withAlpha(0.78f));
    g.drawRoundedRectangle(mixerShell.toFloat(), 12.0f, 1.1f);

    g.setColour(accent.withAlpha(0.28f));
    g.drawLine(18.0f, static_cast<float>(header.getBottom()) - 1.0f,
        static_cast<float>(getWidth()) - 18.0f, static_cast<float>(header.getBottom()) - 1.0f, 1.7f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop(116);

    const int buttonW = 112;
    const int buttonH = 34;
    const int gap = 12;

    auto navShell = juce::Rectangle<int>(getWidth() / 2 - 220, 34, 440, 56).reduced(14, 11);
    int x = navShell.getX();
    const int navY = navShell.getY();

    pulseOneButton.setBounds(x, navY, buttonW, buttonH);
    x += buttonW + gap;
    pulseTwoButton.setBounds(x, navY, buttonW, buttonH);
    x += buttonW + gap;
    dualViewButton.setBounds(x, navY, buttonW, buttonH);

    auto mixerOuter = juce::Rectangle<int>(juce::jmax(getWidth() - 380, getWidth() / 2 + 250), 22, 350, 76);
    aboutButton.setBounds(mixerOuter.getX() - 102, 45, 84, 34);

    auto mixerShell = mixerOuter.reduced(14, 10);
    auto masterRow = mixerShell.removeFromTop(24);
    mixerShell.removeFromTop(6);
    auto crossRow = mixerShell.removeFromTop(24);

    masterVolumeLabel.setBounds(masterRow.removeFromLeft(78));
    masterVolumeSlider.setBounds(masterRow.reduced(4, 1));

    crossfaderLabel.setBounds(crossRow.removeFromLeft(92));
    crossLeftLabel.setVisible(false);
    crossRightLabel.setVisible(false);
    crossfaderSlider.setBounds(crossRow.reduced(4, 1));

    auto content = area.reduced(14);

    if (currentView == ViewMode::PulseOne)
    {
        player1.setCompactMode(false);
        player2.setCompactMode(false);
        player1.setVisible(true);
        player2.setVisible(false);
        player1.setBounds(content);
    }
    else if (currentView == ViewMode::PulseTwo)
    {
        player1.setCompactMode(false);
        player2.setCompactMode(false);
        player1.setVisible(false);
        player2.setVisible(true);
        player2.setBounds(content);
    }
    else
    {
        player1.setCompactMode(true);
        player2.setCompactMode(true);
        player1.setVisible(true);
        player2.setVisible(true);

        const int splitGap = 12;
        const int halfH = (content.getHeight() - splitGap) / 2;
        player1.setBounds(content.removeFromTop(halfH));
        content.removeFromTop(splitGap);
        player2.setBounds(content);
    }

    aboutOverlay.setBounds(getLocalBounds());
    aboutOverlay.setVisible(aboutOverlayOpen);
    if (aboutOverlayOpen)
        aboutOverlay.toFront(false);

    introOverlay.setBounds(getLocalBounds());
    introOverlay.setVisible(introOverlayOpen);
    if (introOverlayOpen)
        introOverlay.toFront(false);
}

void MainComponent::setViewMode(ViewMode newMode)
{
    currentView = newMode;
    updateViewButtons();
    resized();
    repaint();

    juce::MessageManager::callAsync([this, newMode]
        {
            if (newMode == ViewMode::PulseOne)
                player1.promptToRestoreSessionIfAvailable();
            else if (newMode == ViewMode::PulseTwo)
                player2.promptToRestoreSessionIfAvailable();
        });
}

void MainComponent::updateViewButtons()
{
    pulseOneButton.setToggleState(currentView == ViewMode::PulseOne, juce::dontSendNotification);
    pulseTwoButton.setToggleState(currentView == ViewMode::PulseTwo, juce::dontSendNotification);
    dualViewButton.setToggleState(currentView == ViewMode::DualView, juce::dontSendNotification);
}




void MainComponent::timerCallback()
{
    if (introStartAttempted)
    {
        stopTimer();
        return;
    }

    ++introStartupChecks;

    // Wait until the component is attached to the native app window.
    // This makes the embedded browser intro more reliable on startup.
    if (getPeer() == nullptr || getWidth() <= 0 || getHeight() <= 0)
    {
        if (introStartupChecks < 80)
            return;

        // Safety fallback: do not leave the app waiting forever.
    }

    introStartAttempted = true;
    stopTimer();
    startIntroIfAvailable();
}


void MainComponent::startIntroIfAvailable()
{
    const auto introFile = findIntroVideoFile();

    if (!introFile.existsAsFile())
    {
        introOverlayOpen = false;
        introOverlay.setVisible(false);
        return;
    }

    // Make the overlay visible before loading the local HTML video page.
    introOverlayOpen = true;
    introOverlay.setBounds(getLocalBounds());
    introOverlay.setVisible(true);
    introOverlay.toFront(false);
    resized();
    repaint();

    if (introOverlay.startIntro(introFile))
    {
        introOverlayOpen = true;
        introOverlay.setVisible(true);
        introOverlay.toFront(false);
        introOverlay.grabKeyboardFocus();
        resized();
        repaint();
        return;
    }

    introOverlayOpen = true;
    introOverlay.setVisible(true);
    introOverlay.toFront(false);
    introOverlay.showStatusMessage("Intro found but could not load:\n" + introFile.getFullPathName(), 6500);
    resized();
    repaint();
}

void MainComponent::finishIntroOverlay()
{
    introOverlayOpen = false;
    introOverlay.stopIntro();
    introOverlay.setVisible(false);

    // The first restore prompt can be created while the intro is covering the app.
    // Once the intro is gone, refresh the active player's prompt so the user sees it.
    if (currentView == ViewMode::PulseOne)
        player1.promptToRestoreSessionIfAvailable();
    else if (currentView == ViewMode::PulseTwo)
        player2.promptToRestoreSessionIfAvailable();

    resized();
    repaint();
}

juce::File MainComponent::findIntroVideoFile() const
{
    const juce::String folderName = "SonicVibeAssets";
    const juce::String fileName = "intro.mp4";

    auto checkFolder = [&](const juce::File& folder) -> juce::File
        {
            if (!folder.exists())
                return {};

            // 1) Support putting intro.mp4 directly beside SonicVibe.exe.
            auto directCandidate = folder.getChildFile(fileName);
            if (directCandidate.existsAsFile())
                return directCandidate;

            // 2) Support the project asset folder: SonicVibeAssets/intro.mp4.
            auto assetCandidate = folder.getChildFile(folderName).getChildFile(fileName);
            if (assetCandidate.existsAsFile())
                return assetCandidate;

            return {};
        };

    auto checkUpwards = [&](juce::File start) -> juce::File
        {
            for (int i = 0; i < 12 && start.exists(); ++i)
            {
                if (auto found = checkFolder(start); found.existsAsFile())
                    return found;

                auto parent = start.getParentDirectory();
                if (parent == start)
                    break;

                start = parent;
            }

            return {};
        };

    const auto exeFolder = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory();
    if (auto found = checkUpwards(exeFolder); found.existsAsFile())
        return found;

    const auto workingDir = juce::File::getCurrentWorkingDirectory();
    if (auto found = checkUpwards(workingDir); found.existsAsFile())
        return found;

    return {};
}

void MainComponent::showAboutOverlay()
{
    aboutOverlayOpen = true;
    aboutOverlay.setLogoImage(loadAboutLogoImage());
    aboutOverlay.setVisible(true);
    aboutOverlay.toFront(false);
    repaint();
}

void MainComponent::hideAboutOverlay()
{
    aboutOverlayOpen = false;
    aboutOverlay.setVisible(false);
    repaint();
}

juce::Image MainComponent::loadAboutLogoImage() const
{
    const auto logoFile = findAboutLogoFile();
    if (logoFile.existsAsFile())
        return juce::ImageFileFormat::loadFrom(logoFile);

    return {};
}

juce::File MainComponent::findAboutLogoFile() const
{
    const juce::String folderName = "SonicVibeAssets";
    const juce::String fileName = "about_logo.png";

    auto checkUpwards = [&](juce::File start) -> juce::File
        {
            for (int i = 0; i < 9 && start.exists(); ++i)
            {
                auto candidate = start.getChildFile(folderName).getChildFile(fileName);
                if (candidate.existsAsFile())
                    return candidate;

                auto parent = start.getParentDirectory();
                if (parent == start)
                    break;
                start = parent;
            }

            return {};
        };

    if (auto found = checkUpwards(juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory()); found.existsAsFile())
        return found;

    if (auto found = checkUpwards(juce::File::getCurrentWorkingDirectory()); found.existsAsFile())
        return found;

    return {};
}

void MainComponent::styleNavButton(juce::TextButton& button)
{
    button.setClickingTogglesState(false);
    button.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(9, 12, 34));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(38, 181, 255));
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.86f));
    button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
}

void MainComponent::styleMixerSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setColour(juce::Slider::trackColourId, juce::Colour::fromRGB(255, 55, 214));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(38, 231, 255));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromRGB(2, 4, 16));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(5, 7, 24));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGB(95, 82, 190));
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &masterVolumeSlider)
    {
        masterGain.store(static_cast<float>(masterVolumeSlider.getValue()));
        return;
    }

    if (slider == &crossfaderSlider)
    {
        crossfadeAmount.store(static_cast<float>(crossfaderSlider.getValue()));
        return;
    }
}

float MainComponent::getPulseOneCrossfadeGain() const
{
    const float x = juce::jlimit(0.0f, 1.0f, crossfadeAmount.load());
    return std::cos(x * juce::MathConstants<float>::halfPi);
}

float MainComponent::getPulseTwoCrossfadeGain() const
{
    const float x = juce::jlimit(0.0f, 1.0f, crossfadeAmount.load());
    return std::sin(x * juce::MathConstants<float>::halfPi);
}
