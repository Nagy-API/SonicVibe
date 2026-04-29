#include <JuceHeader.h>
#include "PlayerGUI.h"

static juce::String formatTime(double seconds)
{
    int minutes = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}

void PlayerGUI::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerAudio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerGUI::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    playerAudio.getNextAudioBlock(bufferToFill);
}

void PlayerGUI::releaseResources()
{
    playerAudio.releaseResources();
}

PlayerGUI::PlayerGUI()
{
    loadSession();
    thumbnailFormatManager.registerBasicFormats();

    playPauseButton.setButtonText("Play");
    loopButton.setButtonText("Loop: OFF");
    shuffleButton.setButtonText("Shuffle: OFF");
    abLoopButton.setButtonText("A-B Loop");

    for (auto* btn : { &loadButton, &restartButton, &stopButton, &playPauseButton, &startButton, &endButton,
                       &loopButton, &shuffleButton, &forwardButton, &backwardButton,
                       &setAButton, &setBButton, &abLoopButton, &addMarkerButton, &deleteMarkerButton, &muteButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);

        btn->setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(31, 45, 55));
        btn->setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(45, 85, 95));
        btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        btn->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    speedSlider.setRange(0.5, 2.0, 0.1);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);
    addAndMakeVisible(speedSlider);

    for (auto* slider : { &volumeSlider, &positionSlider, &speedSlider })
    {
        slider->setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(80, 190, 210));
        slider->setColour(juce::Slider::trackColourId, juce::Colour::fromRGB(50, 90, 100));
        slider->setColour(juce::Slider::backgroundColourId, juce::Colour::fromRGB(18, 27, 36));
        slider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(20, 25, 35));
        slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGB(90, 110, 125));
    }

    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel.setText("Name: -", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    durationLabel.setFont(juce::Font(13.0f));
    durationLabel.setText("Time: 00:00", juce::dontSendNotification);
    durationLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(durationLabel);

    positionLabel.setFont(juce::Font(13.0f));
    positionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(positionLabel);

    speedLabel.setText("Speed: 1.0x", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(speedLabel);

    playlistBox.updateContent();
    playlistBox.setRowHeight(25);
    playlistBox.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromRGB(12, 18, 25));
    playlistBox.setColour(juce::ListBox::outlineColourId, juce::Colour::fromRGB(70, 95, 105));
    addAndMakeVisible(playlistBox);

    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.addListener(this);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(positionSlider);

    markerModel.markers = &markers;
    markerModel.onMarkerClicked = [this](int row)
        {
            if (row >= 0 && row < (int)markers.size())
            {
                double pos = markers[row].second;
                playerAudio.setPosition(pos);

                if (isPlaying)
                    playerAudio.start();
            }
        };

    markerListBox.setModel(&markerModel);
    markerListBox.updateContent();
    markerListBox.setRowHeight(25);
    markerListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromRGB(12, 18, 25));
    markerListBox.setColour(juce::ListBox::outlineColourId, juce::Colour::fromRGB(70, 95, 105));
    addAndMakeVisible(markerListBox);

    startTimer(50);
}

PlayerGUI::~PlayerGUI()
{
    saveSession();
}

void PlayerGUI::resized()
{
    int margin = 20;
    int buttonW = 90;
    int buttonH = 35;
    int gap = 10;
    int y = 50;

    loadButton.setBounds(margin, y, buttonW, buttonH);
    restartButton.setBounds(margin + (buttonW + gap) * 1, y, buttonW, buttonH);
    stopButton.setBounds(margin + (buttonW + gap) * 2, y, buttonW, buttonH);
    playPauseButton.setBounds(margin + (buttonW + gap) * 3, y, buttonW, buttonH);
    startButton.setBounds(margin + (buttonW + gap) * 4, y, buttonW, buttonH);
    endButton.setBounds(margin + (buttonW + gap) * 5, y, buttonW, buttonH);
    loopButton.setBounds(margin + (buttonW + gap) * 6, y, buttonW, buttonH);
    shuffleButton.setBounds(margin + (buttonW + gap) * 7, y, buttonW + 20, buttonH);
    backwardButton.setBounds(margin + (buttonW + gap) * 8 + 20, y, buttonW, buttonH);
    forwardButton.setBounds(margin + (buttonW + gap) * 9 + 20, y, buttonW, buttonH);

    y += buttonH + 20;

    volumeSlider.setBounds(margin, y, getWidth() / 2.5, 25);
    muteButton.setBounds(margin + getWidth() / 2.5 + 10, y, 90, 25);
    speedLabel.setBounds(margin + getWidth() / 2.5 + 120, y, 100, 25);
    speedSlider.setBounds(margin + getWidth() / 2.5 + 230, y, 150, 25);

    y += 45;

    titleLabel.setBounds(margin, y, getWidth() / 2.5, 25);
    durationLabel.setBounds(margin, y + 25, getWidth() / 2.5, 25);

    int barY = y + 40;
    int barWidth = getWidth() / 2.3;
    positionSlider.setBounds(margin, barY, barWidth, 25);
    positionLabel.setBounds(margin, barY + 25, barWidth, 25);

    int loopY = y + 90;
    setAButton.setBounds(margin, loopY, 80, 30);
    setBButton.setBounds(margin + 90, loopY, 80, 30);
    abLoopButton.setBounds(margin + 180, loopY, 120, 30);
    addMarkerButton.setBounds(margin + 310, loopY, 110, 30);
    deleteMarkerButton.setBounds(margin + 430, loopY, 120, 30);

    int waveY = loopY + 50;
    int waveX = margin;
    int waveW = getWidth() * 0.75;
    int waveH = 120;
    thumbnailArea.setBounds(waveX, waveY, waveW, waveH);

    int listX = getWidth() - 250;
    int listW = 230;
    int listAreaH = (getHeight() - margin * 2 - 20) / 2 - 10;
    int labelH = 24;
    int listBoxH = listAreaH - labelH;

    playlistBox.setBounds(listX, margin + labelH, listW, listBoxH);
    markerListBox.setBounds(listX, margin + listAreaH + 20 + labelH, listW, listBoxH);
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.mp3;*.wav");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();

                if (file.existsAsFile())
                {
                    juce::String ext = file.getFileExtension().toLowerCase();

                    if (ext == ".mp3" || ext == ".wav")
                    {
                        playerAudio.loadFile(file);

                        thumbnail.setSource(new juce::FileInputSource(file));
                        thumbnailLoaded = true;
                        repaint();

                        titleLabel.setText("Name: " + file.getFileNameWithoutExtension(), juce::dontSendNotification);
                        durationLabel.setText("Time: " + playerAudio.getFormattedLength(), juce::dontSendNotification);

                        playlist.push_back(file);
                        playlistBox.updateContent();
                        playlistBox.selectRow((int)playlist.size() - 1);

                        pointA = 0.0;
                        pointB = 0.0;
                        abLoopEnabled = false;
                        playerAudio.enableABLoop(false);
                        setAButton.setButtonText("Set A");
                        setBButton.setButtonText("Set B");
                        abLoopButton.setButtonText("A-B Loop");
                    }
                    else
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Invalid File",
                            "Please select a .mp3 or .wav file.");
                    }
                }
            });

        return;
    }

    if (button == &restartButton)
    {
        playerAudio.stop();
        playerAudio.setPosition(0.0);
        playerAudio.start();
        isPlaying = true;
        playPauseButton.setButtonText("Pause");
        return;
    }

    if (button == &stopButton)
    {
        playerAudio.stop();
        playerAudio.setPosition(0.0);
        isPlaying = false;
        playPauseButton.setButtonText("Play");
        return;
    }

    if (button == &playPauseButton)
    {
        if (!isPlaying)
        {
            if (isShuffle && playlist.size() > 0)
            {
                juce::Random random;
                int currentIndex = playlistBox.getSelectedRow();
                int randomIndex = 0;

                if (playlist.size() == 1)
                {
                    randomIndex = 0;
                }
                else
                {
                    do
                    {
                        randomIndex = random.nextInt((int)playlist.size());
                    } while (randomIndex == currentIndex);
                }

                playerAudio.loadFile(playlist[randomIndex]);
                thumbnail.setSource(new juce::FileInputSource(playlist[randomIndex]));
                thumbnailLoaded = true;
                repaint();

                playlistBox.selectRow(randomIndex);
                titleLabel.setText("Name: " + playlist[randomIndex].getFileNameWithoutExtension(), juce::dontSendNotification);
                durationLabel.setText("Time: " + playerAudio.getFormattedLength(), juce::dontSendNotification);
            }

            playerAudio.start();
            isPlaying = true;
            playPauseButton.setButtonText("Pause");
        }
        else
        {
            playerAudio.stop();
            isPlaying = false;
            playPauseButton.setButtonText("Play");
        }

        return;
    }

    if (button == &startButton)
    {
        playerAudio.setPosition(0.0);

        if (isPlaying)
            playerAudio.start();

        return;
    }

    if (button == &endButton)
    {
        double len = playerAudio.getLength();

        if (len > 0.0)
        {
            playerAudio.setPosition(len);
            playerAudio.stop();
            isPlaying = false;
            playPauseButton.setButtonText("Play");
        }

        return;
    }

    if (button == &muteButton)
    {
        if (!isMuted)
        {
            previousGain = (float)volumeSlider.getValue();
            playerAudio.setGain(0.0f);
            muteButton.setButtonText("Unmute");
            isMuted = true;
        }
        else
        {
            playerAudio.setGain(previousGain);
            muteButton.setButtonText("Mute");
            isMuted = false;
        }

        return;
    }

    if (button == &loopButton)
    {
        isLooping = !isLooping;
        playerAudio.setLooping(isLooping);
        loopButton.setButtonText(isLooping ? "Loop: ON" : "Loop: OFF");
        return;
    }

    if (button == &forwardButton)
    {
        double newPos = playerAudio.getPosition() + 10.0;

        if (newPos < playerAudio.getLength())
            playerAudio.setPosition(newPos);
        else
            playerAudio.setPosition(playerAudio.getLength());

        return;
    }

    if (button == &backwardButton)
    {
        double newPos = playerAudio.getPosition() - 10.0;

        if (newPos > 0.0)
            playerAudio.setPosition(newPos);
        else
            playerAudio.setPosition(0.0);

        return;
    }

    if (button == &setAButton)
    {
        if (playerAudio.getLength() <= 0.0)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "A-B Loop",
                "Load an audio file first.");

            return;
        }

        pointA = playerAudio.getPosition();

        if (pointB > pointA)
            playerAudio.setLoopPoints(pointA, pointB);

        setAButton.setButtonText("A: " + formatTime(pointA));

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Loop A",
            "Start point set at " + formatTime(pointA));

        return;
    }

    if (button == &setBButton)
    {
        if (playerAudio.getLength() <= 0.0)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "A-B Loop",
                "Load an audio file first.");

            return;
        }

        pointB = playerAudio.getPosition();

        if (pointB <= pointA)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "A-B Loop",
                "Point B must be after Point A.");

            return;
        }

        playerAudio.setLoopPoints(pointA, pointB);
        setBButton.setButtonText("B: " + formatTime(pointB));

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Loop B",
            "End point set at " + formatTime(pointB));

        return;
    }

    if (button == &abLoopButton)
    {
        if (playerAudio.getLength() <= 0.0)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "A-B Loop",
                "Load an audio file first.");

            return;
        }

        if (pointB <= pointA)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "A-B Loop",
                "Set point A first, then set point B after it.");

            return;
        }

        abLoopEnabled = !abLoopEnabled;

        playerAudio.setLoopPoints(pointA, pointB);
        playerAudio.enableABLoop(abLoopEnabled);

        if (abLoopEnabled)
        {
            playerAudio.setPosition(pointA);

            if (isPlaying)
                playerAudio.start();

            abLoopButton.setButtonText("A-B Loop ON");
        }
        else
        {
            abLoopButton.setButtonText("A-B Loop OFF");
        }

        return;
    }

    if (button == &shuffleButton)
    {
        isShuffle = !isShuffle;
        shuffleButton.setButtonText(isShuffle ? "Shuffle: ON" : "Shuffle: OFF");
        return;
    }

    if (button == &addMarkerButton)
    {
        if (playerAudio.getLength() <= 0.0)
            return;

        double currentTime = playerAudio.getPosition();
        int markerIndex = (int)markers.size() + 1;

        markers.push_back({ markerIndex, currentTime });

        std::sort(markers.begin(), markers.end(), [](auto& a, auto& b)
            {
                return a.second < b.second;
            });

        markerListBox.updateContent();
        return;
    }

    if (button == &deleteMarkerButton)
    {
        int selectedRow = markerListBox.getSelectedRow();

        if (selectedRow >= 0 && selectedRow < (int)markers.size())
        {
            markers.erase(markers.begin() + selectedRow);
            markerListBox.updateContent();
        }

        return;
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        playerAudio.setGain((float)slider->getValue());
        return;
    }

    if (slider == &positionSlider)
    {
        double len = playerAudio.getLength();

        if (len > 0.0)
        {
            double newPos = positionSlider.getValue() * len;
            playerAudio.setPosition(newPos);
        }

        return;
    }

    if (slider == &speedSlider)
    {
        double ratio = speedSlider.getValue();
        playerAudio.setSpeed(ratio);
        speedLabel.setText("Speed: " + juce::String(ratio, 1) + "x", juce::dontSendNotification);
        return;
    }
}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(14, 20, 30));

    auto headerArea = getLocalBounds().removeFromTop(42);

    g.setColour(juce::Colour::fromRGB(22, 32, 45));
    g.fillRect(headerArea);

    juce::String playerTitle = getName();

    if (playerTitle == "Player 1")
        playerTitle = "Audio Player 1";
    else if (playerTitle == "Player 2")
        playerTitle = "Audio Player 2";
    else if (playerTitle.isEmpty())
        playerTitle = "Simple Audio Player";

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(21.0f, juce::Font::bold));
    g.drawText(playerTitle, headerArea, juce::Justification::centred);

    g.setColour(juce::Colour::fromRGB(65, 85, 100));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f), 10.0f, 2.0f);

    int margin = 20;
    int listX = getWidth() - 250;
    int listW = 230;
    int listAreaH = (getHeight() - margin * 2 - 20) / 2 - 10;
    int labelH = 22;

    g.setColour(juce::Colour::fromRGB(160, 230, 235));
    g.setFont(juce::Font(15.0f, juce::Font::bold));

    g.drawText("Playlist", listX, margin, listW, labelH, juce::Justification::centredLeft);
    g.drawText("Markers", listX, margin + listAreaH + 20, listW, labelH, juce::Justification::centredLeft);

    auto waveformArea = thumbnailArea;

    g.setColour(juce::Colour::fromRGB(38, 45, 52));
    g.fillRoundedRectangle(waveformArea.toFloat(), 8.0f);

    if (thumbnailLoaded && thumbnail.getTotalLength() > 0.0)
    {
        g.setColour(juce::Colour::fromRGB(130, 245, 155));
        thumbnail.drawChannels(
            g,
            waveformArea.reduced(6),
            0.0,
            thumbnail.getTotalLength(),
            1.0f
        );

        double audioPos = playerAudio.getPosition();
        double audioLen = playerAudio.getLength();

        if (audioLen > 0.0)
        {
            double relativeX = audioPos / audioLen;
            int xPos = waveformArea.getX() + (int)(waveformArea.getWidth() * relativeX);

            g.setColour(juce::Colour::fromRGB(255, 80, 80));
            g.drawLine((float)xPos, (float)waveformArea.getY(), (float)xPos, (float)waveformArea.getBottom(), 2.0f);
        }
    }
    else
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(15.0f));
        g.drawText("No waveform loaded", waveformArea, juce::Justification::centred);
    }
}

int PlayerGUI::getNumRows()
{
    return (int)playlist.size();
}

void PlayerGUI::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colour::fromRGB(55, 90, 95));
    else
        g.fillAll(juce::Colour::fromRGB(10, 15, 22));

    g.setColour(juce::Colours::white);

    if (row >= 0 && row < (int)playlist.size())
        g.drawText(playlist[row].getFileName(), 8, 0, width - 10, height, juce::Justification::centredLeft);
}

void PlayerGUI::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < (int)playlist.size())
    {
        playerAudio.loadFile(playlist[row]);

        thumbnail.setSource(new juce::FileInputSource(playlist[row]));
        thumbnailLoaded = true;
        repaint();

        titleLabel.setText("Name: " + playlist[row].getFileNameWithoutExtension(), juce::dontSendNotification);
        durationLabel.setText("Time: " + playerAudio.getFormattedLength(), juce::dontSendNotification);

        pointA = 0.0;
        pointB = 0.0;
        abLoopEnabled = false;
        playerAudio.enableABLoop(false);
        setAButton.setButtonText("Set A");
        setBButton.setButtonText("Set B");
        abLoopButton.setButtonText("A-B Loop");
    }
}

void PlayerGUI::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < (int)playlist.size())
    {
        playerAudio.loadFile(playlist[row]);

        thumbnail.setSource(new juce::FileInputSource(playlist[row]));
        thumbnailLoaded = true;
        repaint();

        titleLabel.setText("Name: " + playlist[row].getFileNameWithoutExtension(), juce::dontSendNotification);
        durationLabel.setText("Time: " + playerAudio.getFormattedLength(), juce::dontSendNotification);

        playerAudio.start();
        isPlaying = true;
        playPauseButton.setButtonText("Pause");
    }
}

void PlayerGUI::timerCallback()
{
    double pos = playerAudio.getPosition();
    double len = playerAudio.getLength();

    if (len > 0.0)
        positionSlider.setValue(pos / len, juce::dontSendNotification);
    else
        positionSlider.setValue(0.0, juce::dontSendNotification);

    positionLabel.setText(
        formatTime(pos) + " / " + formatTime(len),
        juce::dontSendNotification
    );

    if (isShuffle && isPlaying && playlist.size() > 0)
    {
        if (len > 0.0 && pos >= len - 0.2)
        {
            juce::Random random;
            int currentIndex = playlistBox.getSelectedRow();
            int randomIndex = 0;

            if (playlist.size() == 1)
            {
                randomIndex = 0;
            }
            else
            {
                do
                {
                    randomIndex = random.nextInt((int)playlist.size());
                } while (randomIndex == currentIndex);
            }

            playerAudio.loadFile(playlist[randomIndex]);

            thumbnail.setSource(new juce::FileInputSource(playlist[randomIndex]));
            thumbnailLoaded = true;
            repaint();

            playlistBox.selectRow(randomIndex);
            titleLabel.setText("Name: " + playlist[randomIndex].getFileNameWithoutExtension(), juce::dontSendNotification);
            durationLabel.setText("Time: " + playerAudio.getFormattedLength(), juce::dontSendNotification);

            playerAudio.start();
        }
    }

    if (abLoopEnabled)
        playerAudio.checkAndLoopSegment();

    if (thumbnailLoaded)
        repaint();
}

void PlayerGUI::saveSession()
{
    auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("audio_player_session.txt");

    juce::String data;

    if (!playlist.empty())
    {
        auto lastFile = playlist.back().getFullPathName();
        auto pos = playerAudio.getPosition();

        data << "lastFile=" << lastFile << "\n";
        data << "position=" << pos << "\n";
        data << "looping=" << (isLooping ? "1" : "0") << "\n";
    }

    file.replaceWithText(data);
}

void PlayerGUI::loadSession()
{
    auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("audio_player_session.txt");

    if (!file.existsAsFile())
        return;

    auto content = file.loadFileAsString();

    juce::StringArray lines;
    lines.addLines(content);

    juce::String lastFilePath;
    double lastPos = 0.0;
    bool lastLoop = false;

    for (auto& line : lines)
    {
        if (line.startsWith("lastFile="))
            lastFilePath = line.fromFirstOccurrenceOf("=", false, false);
        else if (line.startsWith("position="))
            lastPos = line.fromFirstOccurrenceOf("=", false, false).getDoubleValue();
        else if (line.startsWith("looping="))
            lastLoop = (line.fromFirstOccurrenceOf("=", false, false) == "1");
    }

    if (lastFilePath.isNotEmpty())
    {
        juce::File lastFile(lastFilePath);

        if (lastFile.existsAsFile())
        {
            playerAudio.loadFile(lastFile);
            playerAudio.setPosition(lastPos);
            playerAudio.setLooping(lastLoop);

            titleLabel.setText("Name: " + lastFile.getFileNameWithoutExtension(), juce::dontSendNotification);
            durationLabel.setText("Time: " + playerAudio.getFormattedLength(), juce::dontSendNotification);
        }
    }
}

void PlayerGUI::sliderDragStarted(juce::Slider* slider)
{
    if (slider == &positionSlider)
        stopTimer();
}

void PlayerGUI::sliderDragEnded(juce::Slider* slider)
{
    if (slider == &positionSlider)
        startTimer(50);
}

void PlayerGUI::selectedRowsChanged(int)
{
}