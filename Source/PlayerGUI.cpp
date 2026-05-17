#include <JuceHeader.h>
#include "PlayerGUI.h"


namespace
{
    constexpr const char* kIconPrefix = "sv-icon-";

    bool isIconButton(const juce::Button& button)
    {
        return button.getComponentID().startsWith(kIconPrefix);
    }

    juce::Rectangle<float> iconArea(const juce::Rectangle<int>& bounds)
    {
        auto area = bounds.toFloat().reduced(10.0f, 8.0f);
        const auto size = juce::jmin(area.getWidth(), area.getHeight());
        return area.withSizeKeepingCentre(size, size);
    }

    bool isActiveIconButton(const juce::TextButton& button)
    {
        const auto id = button.getComponentID();
        const auto text = button.getButtonText();

        if (id == "sv-icon-playpause")
            return text.containsIgnoreCase("Pause");

        if (id == "sv-icon-mute")
            return text.containsIgnoreCase("Muted");

        if (id == "sv-icon-loop" || id == "sv-icon-shuffle")
            return text.trim().endsWithIgnoreCase("On");

        if (id == "sv-icon-repeat")
            return !text.containsIgnoreCase("Off");

        return button.getToggleState();
    }

    juce::Colour iconAccentForButton(const juce::TextButton& button)
    {
        const auto id = button.getComponentID();

        if (id == "sv-icon-mute" && isActiveIconButton(button))
            return juce::Colour::fromRGB(255, 92, 132);

        if (id == "sv-icon-shuffle" && isActiveIconButton(button))
            return juce::Colour::fromRGB(184, 104, 255);

        if (id == "sv-icon-repeat" && isActiveIconButton(button))
            return button.getButtonText().containsIgnoreCase("All")
            ? juce::Colour::fromRGB(255, 164, 94)
            : juce::Colour::fromRGB(97, 232, 214);

        if (id == "sv-icon-load")
            return juce::Colour::fromRGB(255, 49, 206);

        return juce::Colour::fromRGB(31, 229, 255);
    }

    bool isPrimaryIconButton(const juce::Button& button)
    {
        return button.getComponentID() == "sv-icon-playpause";
    }

    bool isModeIconButton(const juce::Button& button)
    {
        const auto id = button.getComponentID();
        return id == "sv-icon-mute" || id == "sv-icon-loop" || id == "sv-icon-shuffle" || id == "sv-icon-repeat";
    }

    bool isUtilityIconButton(const juce::Button& button)
    {
        const auto id = button.getComponentID();
        return id == "sv-icon-load" || id == "sv-icon-restart";
    }

    void drawArrowHead(juce::Graphics& g, juce::Point<float> tip, float direction, float size)
    {
        juce::Path p;
        const auto back = tip - juce::Point<float>(std::cos(direction), std::sin(direction)) * size;
        const auto normal = juce::Point<float>(-std::sin(direction), std::cos(direction)) * (size * 0.55f);
        p.addTriangle(tip.x, tip.y, (back + normal).x, (back + normal).y, (back - normal).x, (back - normal).y);
        g.fillPath(p);
    }

    void drawSpeaker(juce::Graphics& g, juce::Rectangle<float> a, bool muted)
    {
        const auto c = a.getCentreY();
        const auto w = a.getWidth();
        const auto h = a.getHeight();

        juce::Path speaker;
        speaker.addRoundedRectangle(a.getX(), c - h * 0.15f, w * 0.22f, h * 0.30f, 2.0f);
        speaker.startNewSubPath(a.getX() + w * 0.20f, c - h * 0.18f);
        speaker.lineTo(a.getX() + w * 0.48f, a.getY() + h * 0.18f);
        speaker.lineTo(a.getX() + w * 0.48f, a.getBottom() - h * 0.18f);
        speaker.lineTo(a.getX() + w * 0.20f, c + h * 0.18f);
        speaker.closeSubPath();
        g.fillPath(speaker);

        juce::PathStrokeType stroke(2.1f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        if (muted)
        {
            const float x0 = a.getX() + w * 0.64f;
            const float x1 = a.getRight() - w * 0.04f;
            g.drawLine(x0, a.getY() + h * 0.26f, x1, a.getBottom() - h * 0.26f, 2.3f);
            g.drawLine(x1, a.getY() + h * 0.26f, x0, a.getBottom() - h * 0.26f, 2.3f);
        }
        else
        {
            // Clear sound waves coming from the speaker's right side, not from above it.
            juce::Path wave1, wave2;
            const float startX = a.getX() + w * 0.56f;
            wave1.startNewSubPath(startX, c - h * 0.23f);
            wave1.cubicTo(a.getX() + w * 0.70f, c - h * 0.13f,
                a.getX() + w * 0.70f, c + h * 0.13f,
                startX, c + h * 0.23f);

            const float startX2 = a.getX() + w * 0.66f;
            wave2.startNewSubPath(startX2, c - h * 0.36f);
            wave2.cubicTo(a.getX() + w * 0.92f, c - h * 0.18f,
                a.getX() + w * 0.92f, c + h * 0.18f,
                startX2, c + h * 0.36f);

            g.strokePath(wave1, stroke);
            g.strokePath(wave2, stroke);
        }
    }

    void drawTransportIcon(juce::Graphics& g, const juce::TextButton& button, juce::Rectangle<int> bounds, bool highlighted)
    {
        const auto id = button.getComponentID();
        auto a = iconArea(bounds);
        const auto accent = iconAccentForButton(button);
        const auto fg = highlighted ? juce::Colours::white : juce::Colours::white.withAlpha(0.82f);
        const bool activeIcon = isActiveIconButton(button);
        const bool modeIcon = isModeIconButton(button);
        g.setColour(activeIcon ? accent : (modeIcon ? juce::Colours::white.withAlpha(highlighted ? 0.82f : 0.58f) : fg));

        if (id == "sv-icon-load")
        {
            auto tray = a.reduced(a.getWidth() * 0.13f, a.getHeight() * 0.12f);
            g.drawRoundedRectangle(tray.withTrimmedTop(a.getHeight() * 0.58f), 2.0f, 2.0f);
            g.drawLine(a.getCentreX(), a.getY() + 2.0f, a.getCentreX(), a.getCentreY() + 3.0f, 2.4f);
            drawArrowHead(g, { a.getCentreX(), a.getCentreY() + 5.0f }, juce::MathConstants<float>::halfPi, 7.0f);
            return;
        }

        if (id == "sv-icon-playpause")
        {
            if (button.getButtonText().containsIgnoreCase("Pause"))
            {
                const auto w = a.getWidth() * 0.22f;
                g.fillRoundedRectangle(a.getX() + a.getWidth() * 0.20f, a.getY() + 2.0f, w, a.getHeight() - 4.0f, 2.0f);
                g.fillRoundedRectangle(a.getRight() - a.getWidth() * 0.20f - w, a.getY() + 2.0f, w, a.getHeight() - 4.0f, 2.0f);
            }
            else
            {
                juce::Path p;
                p.addTriangle(a.getX() + a.getWidth() * 0.28f, a.getY() + 1.0f,
                    a.getX() + a.getWidth() * 0.28f, a.getBottom() - 1.0f,
                    a.getRight() - a.getWidth() * 0.18f, a.getCentreY());
                g.fillPath(p);
            }
            return;
        }

        if (id == "sv-icon-stop")
        {
            g.fillRoundedRectangle(a.reduced(a.getWidth() * 0.18f), 2.0f);
            return;
        }

        if (id == "sv-icon-prev" || id == "sv-icon-next" || id == "sv-icon-start" || id == "sv-icon-end")
        {
            const bool right = (id == "sv-icon-next" || id == "sv-icon-end");
            if (right)
            {
                juce::Path t1, t2;
                t1.addTriangle(a.getX() + a.getWidth() * 0.16f, a.getY() + 2.0f,
                    a.getX() + a.getWidth() * 0.16f, a.getBottom() - 2.0f,
                    a.getX() + a.getWidth() * 0.56f, a.getCentreY());
                t2.addTriangle(a.getX() + a.getWidth() * 0.46f, a.getY() + 2.0f,
                    a.getX() + a.getWidth() * 0.46f, a.getBottom() - 2.0f,
                    a.getRight() - a.getWidth() * 0.14f, a.getCentreY());
                if (id == "sv-icon-end")
                    g.fillRoundedRectangle(a.getRight() - 3.0f, a.getY() + 1.0f, 3.0f, a.getHeight() - 2.0f, 1.5f);
                g.fillPath(t1); g.fillPath(t2);
            }
            else
            {
                juce::Path t1, t2;
                t1.addTriangle(a.getRight() - a.getWidth() * 0.16f, a.getY() + 2.0f,
                    a.getRight() - a.getWidth() * 0.16f, a.getBottom() - 2.0f,
                    a.getX() + a.getWidth() * 0.44f, a.getCentreY());
                t2.addTriangle(a.getRight() - a.getWidth() * 0.46f, a.getY() + 2.0f,
                    a.getRight() - a.getWidth() * 0.46f, a.getBottom() - 2.0f,
                    a.getX() + a.getWidth() * 0.14f, a.getCentreY());
                if (id == "sv-icon-start")
                    g.fillRoundedRectangle(a.getX(), a.getY() + 1.0f, 3.0f, a.getHeight() - 2.0f, 1.5f);
                g.fillPath(t1); g.fillPath(t2);
            }
            return;
        }

        if (id == "sv-icon-restart")
        {
            juce::Path p;
            p.addCentredArc(a.getCentreX(), a.getCentreY(), a.getWidth() * 0.36f, a.getHeight() * 0.36f, 0.0f,
                -0.2f, juce::MathConstants<float>::twoPi * 0.78f, true);
            g.strokePath(p, juce::PathStrokeType(2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            drawArrowHead(g, { a.getX() + a.getWidth() * 0.22f, a.getY() + a.getHeight() * 0.24f }, -2.3f, 6.0f);
            return;
        }

        if (id == "sv-icon-back10" || id == "sv-icon-forward10")
        {
            const bool forward = id == "sv-icon-forward10";
            juce::Path p;
            const auto cy = a.getCentreY() - 1.0f;
            if (forward)
            {
                p.startNewSubPath(a.getX() + 3.0f, cy);
                p.lineTo(a.getRight() - 12.0f, cy);
                g.strokePath(p, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                drawArrowHead(g, { a.getRight() - 5.0f, cy }, 0.0f, 7.0f);
            }
            else
            {
                p.startNewSubPath(a.getRight() - 3.0f, cy);
                p.lineTo(a.getX() + 12.0f, cy);
                g.strokePath(p, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                drawArrowHead(g, { a.getX() + 5.0f, cy }, juce::MathConstants<float>::pi, 7.0f);
            }
            g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
            g.drawText("10", a.toNearestInt().withTrimmedTop(static_cast<int>(a.getHeight() * 0.42f)), juce::Justification::centred);
            return;
        }

        if (id == "sv-icon-mute")
        {
            const bool muted = button.getButtonText().containsIgnoreCase("Muted");
            auto full = bounds.toFloat().reduced(9.0f, 6.0f);
            auto speaker = full.withWidth(34.0f).withHeight(24.0f);
            speaker.setCentre(full.getX() + 17.0f, full.getCentreY());

            g.setColour(muted ? juce::Colour::fromRGB(255, 105, 115)
                : juce::Colour::fromRGB(82, 228, 255).withAlpha(highlighted ? 1.0f : 0.86f));
            drawSpeaker(g, speaker, muted);

            if (bounds.getWidth() >= 82)
            {
                g.setColour(muted ? juce::Colours::white.withAlpha(0.92f)
                    : juce::Colours::white.withAlpha(0.72f));
                g.setFont(juce::Font(juce::FontOptions(11.5f, juce::Font::bold)));
                g.drawFittedText(muted ? "Muted" : "Sound",
                    bounds.withTrimmedLeft(44).reduced(4, 2),
                    juce::Justification::centredLeft, 1, 0.82f);
            }
            return;
        }

        if (id == "sv-icon-loop" || id == "sv-icon-repeat")
        {
            auto r = a.reduced(2.0f);
            juce::Path top, bottom;
            top.startNewSubPath(r.getX() + 3.0f, r.getCentreY() - 5.0f);
            top.lineTo(r.getRight() - 9.0f, r.getCentreY() - 5.0f);
            bottom.startNewSubPath(r.getRight() - 3.0f, r.getCentreY() + 5.0f);
            bottom.lineTo(r.getX() + 9.0f, r.getCentreY() + 5.0f);
            g.strokePath(top, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.strokePath(bottom, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            drawArrowHead(g, { r.getRight() - 3.0f, r.getCentreY() - 5.0f }, 0.0f, 5.5f);
            drawArrowHead(g, { r.getX() + 3.0f, r.getCentreY() + 5.0f }, juce::MathConstants<float>::pi, 5.5f);
            if (id == "sv-icon-repeat")
            {
                g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
                g.drawText(button.getButtonText().containsIgnoreCase("All") ? "A" : "1",
                    a.toNearestInt(), juce::Justification::centred);
            }
            return;
        }

        if (id == "sv-icon-shuffle")
        {
            auto r = a.reduced(3.0f);
            juce::Path a1, a2;
            a1.startNewSubPath(r.getX(), r.getY() + r.getHeight() * 0.28f);
            a1.cubicTo(r.getCentreX(), r.getY() + r.getHeight() * 0.28f, r.getCentreX(), r.getBottom() - r.getHeight() * 0.28f, r.getRight() - 7.0f, r.getBottom() - r.getHeight() * 0.28f);
            a2.startNewSubPath(r.getX(), r.getBottom() - r.getHeight() * 0.28f);
            a2.cubicTo(r.getCentreX(), r.getBottom() - r.getHeight() * 0.28f, r.getCentreX(), r.getY() + r.getHeight() * 0.28f, r.getRight() - 7.0f, r.getY() + r.getHeight() * 0.28f);
            g.strokePath(a1, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.strokePath(a2, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            drawArrowHead(g, { r.getRight(), r.getBottom() - r.getHeight() * 0.28f }, 0.0f, 5.5f);
            drawArrowHead(g, { r.getRight(), r.getY() + r.getHeight() * 0.28f }, 0.0f, 5.5f);
            return;
        }
    }

    class SonicIconButtonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
            const juce::Colour&, bool isMouseOverButton, bool isButtonDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
            auto* textButton = dynamic_cast<juce::TextButton*>(&button);

            const bool primary = isPrimaryIconButton(button);
            const bool mode = isModeIconButton(button);
            const bool utility = isUtilityIconButton(button);
            const bool active = textButton != nullptr && isActiveIconButton(*textButton);

            const auto accent = textButton != nullptr ? iconAccentForButton(*textButton) : juce::Colour::fromRGB(95, 220, 230);
            const auto neutralTop = utility ? juce::Colour::fromRGB(70, 30, 86)
                : (mode ? juce::Colour::fromRGB(22, 18, 48) : juce::Colour::fromRGB(29, 22, 58));
            const auto neutralBottom = utility ? juce::Colour::fromRGB(8, 8, 24)
                : (mode ? juce::Colour::fromRGB(6, 7, 22) : juce::Colour::fromRGB(7, 8, 24));
            const auto activeTop = accent.withMultipliedBrightness(primary ? 0.90f : 0.70f);
            const auto activeBottom = accent.withMultipliedBrightness(primary ? 0.40f : 0.32f);
            const auto top = active || primary ? activeTop : neutralTop;
            const auto bottom = active || primary ? activeBottom : neutralBottom;

            const float corner = primary ? bounds.getHeight() * 0.50f : (mode ? 12.0f : 8.0f);
            const float glowAlpha = primary ? 0.42f : (active ? 0.25f : (mode ? 0.0f : 0.08f));

            g.setColour(juce::Colours::black.withAlpha(0.26f));
            g.fillRoundedRectangle(bounds.translated(0.0f, 2.0f), corner);

            if (primary || active)
            {
                g.setColour(accent.withAlpha(glowAlpha));
                g.fillRoundedRectangle(bounds.expanded(primary ? 5.0f : 2.0f), corner + 5.0f);
            }

            if (isButtonDown)
                bounds = bounds.reduced(1.0f);

            juce::ColourGradient grad(top.brighter(isMouseOverButton ? 0.10f : 0.00f), bounds.getX(), bounds.getY(),
                bottom.brighter(isMouseOverButton ? 0.08f : 0.00f), bounds.getX(), bounds.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, corner);

            g.setColour(juce::Colours::white.withAlpha(primary ? 0.25f : 0.12f));
            g.drawLine(bounds.getX() + 6.0f, bounds.getY() + 1.0f, bounds.getRight() - 6.0f, bounds.getY() + 1.0f, 1.0f);

            g.setColour((active || primary ? accent : juce::Colour::fromRGB(145, 122, 180)).withAlpha(isMouseOverButton ? 0.95f : 0.72f));
            g.drawRoundedRectangle(bounds, corner, (active || primary) ? 1.7f : 1.0f);
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& button,
            bool isMouseOverButton, bool isButtonDown) override
        {
            if (isIconButton(button))
            {
                drawTransportIcon(g, button, button.getLocalBounds(), isMouseOverButton || isButtonDown);
                return;
            }

            g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                : juce::TextButton::textColourOffId));
            g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::plain)));
            g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(6, 2),
                juce::Justification::centred, 1, 0.85f);
        }
    };

    SonicIconButtonLookAndFeel sonicIconButtonLookAndFeel;
}

static juce::String formatTime(double seconds)
{
    const auto safeSeconds = juce::jmax(0.0, seconds);
    const int minutes = static_cast<int>(safeSeconds) / 60;
    const int secs = static_cast<int>(safeSeconds) % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}




RestoreSessionOverlay::RestoreSessionOverlay()
{
    setInterceptsMouseClicks(true, true);

    addAndMakeVisible(restoreButton);
    addAndMakeVisible(startEmptyButton);

    restoreButton.setTooltip("Restore the saved playlist for this pulse");
    startEmptyButton.setTooltip("Ignore the saved playlist and start with an empty playlist");
    restoreButton.setLookAndFeel(&sonicIconButtonLookAndFeel);
    startEmptyButton.setLookAndFeel(&sonicIconButtonLookAndFeel);

    auto style = [](juce::TextButton& button, juce::Colour base, juce::Colour active)
        {
            button.setColour(juce::TextButton::buttonColourId, base);
            button.setColour(juce::TextButton::buttonOnColourId, active);
            button.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.95f));
            button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        };

    style(restoreButton, juce::Colour::fromRGB(18, 82, 118), juce::Colour::fromRGB(31, 190, 240));
    style(startEmptyButton, juce::Colour::fromRGB(48, 24, 82), juce::Colour::fromRGB(202, 72, 220));
}

void RestoreSessionOverlay::setPulseTitle(const juce::String& newTitle)
{
    pulseTitle = newTitle.isNotEmpty() ? newTitle : "Pulse";
    repaint();
}

void RestoreSessionOverlay::resized()
{
    auto card = juce::Rectangle<int>(520, 240).withCentre(getLocalBounds().getCentre().translated(0, -78));
    auto content = card.reduced(30, 24);

    content.removeFromTop(134);
    auto row = content.removeFromTop(44);
    const int gap = 14;
    const int w = (row.getWidth() - gap) / 2;
    restoreButton.setBounds(row.removeFromLeft(w));
    row.removeFromLeft(gap);
    startEmptyButton.setBounds(row.removeFromLeft(w));
}

void RestoreSessionOverlay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds();

    g.fillAll(juce::Colours::black.withAlpha(0.64f));

    auto card = juce::Rectangle<int>(520, 240).withCentre(bounds.getCentre().translated(0, -78)).toFloat();
    const auto accent = juce::Colour::fromRGB(31, 229, 255);
    const auto accentPink = juce::Colour::fromRGB(255, 49, 206);
    const auto accentViolet = juce::Colour::fromRGB(112, 58, 255);

    g.setColour(juce::Colours::black.withAlpha(0.42f));
    g.fillRoundedRectangle(card.translated(0.0f, 10.0f), 24.0f);

    juce::ColourGradient grad(juce::Colour::fromRGB(15, 20, 58), card.getX(), card.getY(),
        juce::Colour::fromRGB(4, 5, 20), card.getRight(), card.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(card, 24.0f);

    juce::ColourGradient wash(accent.withAlpha(0.10f), card.getX() + 40.0f, card.getY() + 30.0f,
        accentViolet.withAlpha(0.10f), card.getRight() - 50.0f, card.getBottom() - 40.0f, false);
    g.setGradientFill(wash);
    g.fillRoundedRectangle(card.reduced(12.0f), 20.0f);

    g.setColour(accent.withAlpha(0.82f));
    g.drawRoundedRectangle(card, 24.0f, 1.5f);
    g.setColour(accentPink.withAlpha(0.22f));
    g.drawRoundedRectangle(card.reduced(3.0f), 21.0f, 0.9f);

    auto text = card.toNearestInt().reduced(34, 24);

    auto icon = text.removeFromTop(48).withSizeKeepingCentre(44, 44).toFloat();
    g.setColour(accent.withAlpha(0.16f));
    g.fillEllipse(icon.expanded(6.0f));
    g.setColour(accent.withAlpha(0.95f));
    g.fillEllipse(icon);
    g.setColour(juce::Colour::fromRGB(8, 16, 28));
    g.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)));
    g.drawText("SV", icon.toNearestInt(), juce::Justification::centred);

    text.removeFromTop(6);
    g.setColour(accent);
    g.setFont(juce::Font(juce::FontOptions(20.5f, juce::Font::bold)));
    g.drawText("Saved playlist found", text.removeFromTop(30), juce::Justification::centred);

    text.removeFromTop(6);
    g.setColour(juce::Colours::white.withAlpha(0.82f));
    g.setFont(juce::Font(juce::FontOptions(13.6f)));
    g.drawFittedText("Restore the saved playlist for " + pulseTitle + " or start with a clean empty playlist.",
        text.removeFromTop(44), juce::Justification::centred, 2, 0.90f);

    auto buttonPlate = card.reduced(20.0f).withTrimmedTop(176.0f).withHeight(48.0f);
    g.setColour(juce::Colours::black.withAlpha(0.16f));
    g.fillRoundedRectangle(buttonPlate, 16.0f);
}


static int readBigEndianInt(const juce::uint8* bytes)
{
    return (static_cast<int>(bytes[0]) << 24)
        | (static_cast<int>(bytes[1]) << 16)
        | (static_cast<int>(bytes[2]) << 8)
        | static_cast<int>(bytes[3]);
}

static int readSynchsafeInt(const juce::uint8* bytes)
{
    return ((static_cast<int>(bytes[0]) & 0x7f) << 21)
        | ((static_cast<int>(bytes[1]) & 0x7f) << 14)
        | ((static_cast<int>(bytes[2]) & 0x7f) << 7)
        | (static_cast<int>(bytes[3]) & 0x7f);
}

static juce::Image loadImageFromMemoryRange(const juce::uint8* data, size_t size)
{
    if (data == nullptr || size < 16)
        return {};

    juce::MemoryInputStream imageStream(data, size, false);
    return juce::ImageFileFormat::loadFrom(imageStream);
}

static juce::Image tryLoadImageFromEmbeddedFrameBytes(const juce::uint8* data, size_t size)
{
    if (data == nullptr || size < 16)
        return {};

    for (size_t i = 0; i + 16 < size; ++i)
    {
        const bool looksLikeJpeg = data[i] == 0xff && data[i + 1] == 0xd8 && data[i + 2] == 0xff;
        const bool looksLikePng = data[i] == 0x89 && data[i + 1] == 0x50 && data[i + 2] == 0x4e && data[i + 3] == 0x47
            && data[i + 4] == 0x0d && data[i + 5] == 0x0a && data[i + 6] == 0x1a && data[i + 7] == 0x0a;

        if (looksLikeJpeg || looksLikePng)
        {
            auto image = loadImageFromMemoryRange(data + i, size - i);
            if (image.isValid())
                return image;
        }
    }

    return {};
}

static juce::Image tryLoadID3v2AlbumArt(const juce::File& audioFile)
{
    if (!audioFile.existsAsFile())
        return {};

    juce::FileInputStream stream(audioFile);
    if (!stream.openedOk())
        return {};

    juce::uint8 header[10]{};
    if (stream.read(header, 10) != 10)
        return {};

    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
        return {};

    const int majorVersion = header[3];
    if (majorVersion < 3 || majorVersion > 4)
        return {};

    const int tagSize = readSynchsafeInt(header + 6);
    if (tagSize <= 0 || tagSize > 32 * 1024 * 1024)
        return {};

    juce::MemoryBlock tagData;
    tagData.setSize(static_cast<size_t>(tagSize));
    if (stream.read(tagData.getData(), tagSize) <= 0)
        return {};

    const auto* bytes = static_cast<const juce::uint8*>(tagData.getData());
    size_t offset = 0;
    const size_t total = tagData.getSize();

    const bool hasExtendedHeader = (header[5] & 0x40) != 0;
    if (hasExtendedHeader && total >= 4)
    {
        const int extSize = (majorVersion == 4) ? readSynchsafeInt(bytes) : readBigEndianInt(bytes);
        if (extSize > 0 && static_cast<size_t>(extSize) < total)
            offset = (majorVersion == 3) ? static_cast<size_t>(extSize + 4) : static_cast<size_t>(extSize);
    }

    while (offset + 10 <= total)
    {
        const auto* frame = bytes + offset;
        if (frame[0] == 0 || frame[1] == 0 || frame[2] == 0 || frame[3] == 0)
            break;

        const bool isAPICFrame = frame[0] == 'A' && frame[1] == 'P' && frame[2] == 'I' && frame[3] == 'C';
        const int frameSize = (majorVersion == 4) ? readSynchsafeInt(frame + 4) : readBigEndianInt(frame + 4);
        if (frameSize <= 0)
            break;

        const size_t frameStart = offset + 10;
        const size_t frameEnd = frameStart + static_cast<size_t>(frameSize);
        if (frameEnd > total)
            break;

        if (isAPICFrame)
        {
            const auto* frameData = bytes + frameStart;
            const size_t frameBytes = static_cast<size_t>(frameSize);

            // Robust fallback: search the APIC frame for JPEG/PNG magic. This avoids depending on
            // every possible text-encoding/description variation in ID3 tags.
            auto image = tryLoadImageFromEmbeddedFrameBytes(frameData, frameBytes);
            if (image.isValid())
                return image;

            // Conservative parser for normal APIC frames: encoding, mime, picture type, description, image bytes.
            size_t p = 0;
            if (frameBytes < 5)
                return {};

            const juce::uint8 encoding = frameData[p++];
            while (p < frameBytes && frameData[p] != 0)
                ++p;
            if (p >= frameBytes)
                return {};
            ++p; // mime terminator

            if (p >= frameBytes)
                return {};
            ++p; // picture type

            if (encoding == 1 || encoding == 2) // UTF-16 terminator
            {
                while (p + 1 < frameBytes && !(frameData[p] == 0 && frameData[p + 1] == 0))
                    ++p;
                p = juce::jmin(p + 2, frameBytes);
            }
            else
            {
                while (p < frameBytes && frameData[p] != 0)
                    ++p;
                p = juce::jmin(p + 1, frameBytes);
            }

            image = loadImageFromMemoryRange(frameData + p, frameBytes - p);
            if (image.isValid())
                return image;
        }

        offset = frameEnd;
    }

    return {};
}

static bool parseLrcTimestamp(const juce::String& rawTimestamp, double& outSeconds)
{
    const auto timestamp = rawTimestamp.trim();
    const int colon = timestamp.indexOfChar(':');

    if (colon <= 0)
        return false;

    const int minutes = timestamp.substring(0, colon).getIntValue();
    const double seconds = timestamp.substring(colon + 1).getDoubleValue();

    if (minutes < 0 || seconds < 0.0)
        return false;

    outSeconds = static_cast<double>(minutes * 60) + seconds;
    return true;
}

PlayerGUI::PlayerGUI()
{
    thumbnailFormatManager.registerBasicFormats();
    setWantsKeyboardFocus(true);

    auto buttons = { &playlistToggleButton, &lyricsToggleButton, &markerToggleButton, &trackInfoToggleButton,
                     &loadButton, &loadQueueButton, &saveQueueButton, &removeTrackButton, &clearQueueButton, &moveUpButton, &moveDownButton,
                     &autoLyricsButton, &loadLyricsButton, &clearLyricsButton,
                     &playPauseButton, &stopButton, &restartButton, &previousTrackButton, &nextTrackButton,
                     &startButton, &endButton, &backwardButton, &forwardButton,
                     &muteButton, &loopButton, &shuffleButton, &repeatOneButton,
                     &setAButton, &setBButton, &abLoopButton, &addMarkerButton, &deleteMarkerButton };

    for (auto* button : buttons)
    {
        button->addListener(this);
        addAndMakeVisible(button);

        button->setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(16, 16, 42));
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(42, 154, 220));
        button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        button->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }

    restoreOverlay.restoreButton.addListener(this);
    restoreOverlay.startEmptyButton.addListener(this);
    restoreOverlay.setVisible(false);
    restoreOverlay.setAlwaysOnTop(true);
    addAndMakeVisible(restoreOverlay);
    playlistToggleButton.setVisible(false);
    lyricsToggleButton.setVisible(false);
    markerToggleButton.setVisible(false);
    trackInfoToggleButton.setVisible(false);
    playlistToggleButton.setTooltip("Show playlist and playlist tools");
    lyricsToggleButton.setTooltip("Show synced lyrics");
    markerToggleButton.setTooltip("Show cue markers and marker tools");
    trackInfoToggleButton.setTooltip("Show current track details");
    autoLyricsButton.setButtonText(autoLyricsEnabled ? "Auto Lyrics On" : "Auto Lyrics Off");

    auto configureIconButton = [](juce::TextButton& button, const juce::String& id, const juce::String& tooltip)
        {
            button.setComponentID(id);
            button.setTooltip(tooltip);
            button.setLookAndFeel(&sonicIconButtonLookAndFeel);
        };

    configureIconButton(loadButton, "sv-icon-load", "Load audio files");
    configureIconButton(playPauseButton, "sv-icon-playpause", "Play / Pause");
    configureIconButton(stopButton, "sv-icon-stop", "Stop");
    configureIconButton(restartButton, "sv-icon-restart", "Restart current track");
    configureIconButton(previousTrackButton, "sv-icon-prev", "Previous track");
    configureIconButton(nextTrackButton, "sv-icon-next", "Next track");
    configureIconButton(startButton, "sv-icon-start", "Jump to start");
    configureIconButton(endButton, "sv-icon-end", "Jump to end");
    configureIconButton(backwardButton, "sv-icon-back10", "Back 10 seconds");
    configureIconButton(forwardButton, "sv-icon-forward10", "Forward 10 seconds");
    configureIconButton(muteButton, "sv-icon-mute", "Mute or unmute this pulse");
    muteButton.setButtonText("Sound");
    configureIconButton(loopButton, "sv-icon-loop", "Loop current track instantly at the audio layer");
    configureIconButton(shuffleButton, "sv-icon-shuffle", "Shuffle playlist order when auto-next runs");
    configureIconButton(repeatOneButton, "sv-icon-repeat", "Repeat mode: Off / One / All");
    updateRepeatButtonVisuals();

    auto styleButton = [](juce::TextButton& button, juce::Colour base, juce::Colour active)
        {
            button.setColour(juce::TextButton::buttonColourId, base);
            button.setColour(juce::TextButton::buttonOnColourId, active);
            button.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.92f));
            button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        };

    // Sidebar tabs: same system, different accents so the section has life without noise.
    styleButton(playlistToggleButton, juce::Colour::fromRGB(18, 17, 46), juce::Colour::fromRGB(31, 150, 220));
    styleButton(lyricsToggleButton, juce::Colour::fromRGB(18, 17, 46), juce::Colour::fromRGB(196, 62, 232));
    styleButton(markerToggleButton, juce::Colour::fromRGB(18, 17, 46), juce::Colour::fromRGB(112, 74, 255));
    styleButton(trackInfoToggleButton, juce::Colour::fromRGB(18, 17, 46), juce::Colour::fromRGB(255, 49, 206));

    // Utility/action buttons get clearer roles.
    styleButton(loadQueueButton, juce::Colour::fromRGB(25, 35, 68), juce::Colour::fromRGB(76, 178, 220));
    styleButton(saveQueueButton, juce::Colour::fromRGB(32, 42, 66), juce::Colour::fromRGB(94, 220, 196));
    styleButton(removeTrackButton, juce::Colour::fromRGB(82, 34, 70), juce::Colour::fromRGB(230, 72, 130));
    styleButton(clearQueueButton, juce::Colour::fromRGB(82, 34, 70), juce::Colour::fromRGB(230, 72, 130));
    styleButton(moveUpButton, juce::Colour::fromRGB(34, 35, 68), juce::Colour::fromRGB(95, 92, 205));
    styleButton(moveDownButton, juce::Colour::fromRGB(34, 35, 68), juce::Colour::fromRGB(95, 92, 205));

    styleButton(setAButton, juce::Colour::fromRGB(28, 48, 67), juce::Colour::fromRGB(63, 164, 194));
    styleButton(setBButton, juce::Colour::fromRGB(28, 48, 67), juce::Colour::fromRGB(63, 164, 194));
    styleButton(abLoopButton, juce::Colour::fromRGB(72, 49, 38), juce::Colour::fromRGB(225, 139, 73));
    styleButton(addMarkerButton, juce::Colour::fromRGB(31, 42, 75), juce::Colour::fromRGB(95, 123, 225));
    styleButton(deleteMarkerButton, juce::Colour::fromRGB(72, 32, 58), juce::Colour::fromRGB(210, 78, 116));

    styleButton(autoLyricsButton, juce::Colour::fromRGB(24, 54, 72), juce::Colour::fromRGB(82, 210, 236));
    styleButton(loadLyricsButton, juce::Colour::fromRGB(36, 30, 76), juce::Colour::fromRGB(134, 104, 236));
    styleButton(clearLyricsButton, juce::Colour::fromRGB(82, 34, 70), juce::Colour::fromRGB(230, 72, 130));

    loadQueueButton.setTooltip("Import a saved playlist file (.svpl/.m3u/.txt)");
    saveQueueButton.setTooltip("Export the current playlist to a file so you can reuse it later");
    removeTrackButton.setTooltip("Remove the selected track from the playlist");
    clearQueueButton.setTooltip("Clear all tracks from the playlist");
    moveUpButton.setTooltip("Move the selected track up");
    moveDownButton.setTooltip("Move the selected track down");
    autoLyricsButton.setTooltip("Automatically load matching lyrics when available");
    loadLyricsButton.setTooltip("Load lyrics file manually");
    clearLyricsButton.setTooltip("Clear loaded lyrics");
    setAButton.setTooltip("Set A point for A-B loop");
    setBButton.setTooltip("Set B point for A-B loop");
    abLoopButton.setTooltip("Toggle A-B loop");
    addMarkerButton.setTooltip("Add cue marker at current position");
    deleteMarkerButton.setTooltip("Delete selected cue marker");


    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setTooltip("Pulse volume");
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSlider.addListener(this);
    addAndMakeVisible(positionSlider);

    // SoundTouch tempo control: changes playback speed without changing pitch.
    speedSlider.setRange(0.50, 2.00, 0.01);
    speedSlider.setValue(1.0, juce::dontSendNotification);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 86, 22);
    speedSlider.setEnabled(true);
    speedSlider.setTooltip("Tempo control: 0.5x to 2.0x without changing pitch. Extreme values may reduce quality.");
    speedSlider.addListener(this);
    speedSlider.onValueChange = [this] { applyTempoFromSlider(); };
    addAndMakeVisible(speedSlider);

    for (auto* slider : { &volumeSlider, &positionSlider, &speedSlider })
    {
        slider->setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(31, 229, 255));
        slider->setColour(juce::Slider::trackColourId, juce::Colour::fromRGB(255, 49, 206));
        slider->setColour(juce::Slider::backgroundColourId, juce::Colour::fromRGB(7, 8, 24));
        slider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(11, 10, 31));
        slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGB(106, 86, 168));
    }

    titleLabel.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setMinimumHorizontalScale(0.65f);
    addAndMakeVisible(titleLabel);

    durationLabel.setFont(juce::Font(juce::FontOptions(13.0f)));
    durationLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(durationLabel);

    positionLabel.setFont(juce::Font(juce::FontOptions(13.0f)));
    positionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(positionLabel);

    speedLabel.setFont(juce::Font(juce::FontOptions(13.0f)));
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(speedLabel);

    queueStatusLabel.setFont(juce::Font(juce::FontOptions(12.5f, juce::Font::bold)));
    queueStatusLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(95, 220, 230));
    addAndMakeVisible(queueStatusLabel);

    lyricsStatusLabel.setFont(juce::Font(juce::FontOptions(12.5f, juce::Font::bold)));
    lyricsStatusLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(95, 220, 230));
    addAndMakeVisible(lyricsStatusLabel);

    playlistBox.updateContent();
    playlistBox.setRowHeight(32);
    playlistBox.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromRGB(4, 4, 14));
    playlistBox.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(playlistBox);

    markerModel.markers = &markers;
    markerModel.onMarkerClicked = [this](int row)
        {
            if (row >= 0 && row < static_cast<int>(markers.size()))
            {
                setSyncedPlaybackPosition(markers[static_cast<size_t>(row)].second);

                if (isPlaying)
                    playerAudio.start();
            }
        };

    markerListBox.setModel(&markerModel);
    markerListBox.updateContent();
    markerListBox.setRowHeight(30);
    markerListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromRGB(4, 4, 14));
    markerListBox.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(markerListBox);

    lyricsModel.lyrics = &lyrics;
    lyricsModel.currentIndex = &currentLyricIndex;
    lyricsModel.onLyricClicked = [this](int row)
        {
            seekToLyricLine(row);
        };
    lyricsListBox.setModel(&lyricsModel);
    lyricsListBox.updateContent();
    lyricsListBox.setRowHeight(76);
    lyricsListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromRGB(4, 4, 14));
    lyricsListBox.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(lyricsListBox);

    refreshButtonVisibility();
    updateQueueStatusLabel();
    updateLyricsStatusLabel();
    startTimer(50);
}

PlayerGUI::~PlayerGUI()
{
    auto buttons = { &loadButton, &playPauseButton, &stopButton, &restartButton, &previousTrackButton, &nextTrackButton,
                     &startButton, &endButton, &backwardButton, &forwardButton, &muteButton, &loopButton,
                     &shuffleButton, &repeatOneButton };

    for (auto* button : buttons)
        button->setLookAndFeel(nullptr);

    restoreOverlay.restoreButton.removeListener(this);
    restoreOverlay.startEmptyButton.removeListener(this);

    saveSession();
}

void PlayerGUI::setPulseTitle(const juce::String& newTitle)
{
    pulseTitle = newTitle.isNotEmpty() ? newTitle : "Pulse";
    setName(pulseTitle);
    restoreOverlay.setPulseTitle(pulseTitle);
    repaint();
}

void PlayerGUI::promptToRestoreSessionIfAvailable()
{
    // The startup intro can temporarily sit above the player. If the restore
    // prompt was already prepared behind it, calling this again should simply
    // bring the existing prompt back to the front instead of ignoring it.
    if (sessionLoaded)
    {
        if (restorePromptVisible)
        {
            updateRestorePromptVisibility();
            resized();
            repaint();
        }

        return;
    }

    sessionLoaded = true;

    restorePromptVisible = hasRestorableSession();

    if (!restorePromptVisible)
        return;

    updateRestorePromptVisibility();
    resized();
    repaint();
}


void PlayerGUI::updateRestorePromptVisibility()
{
    const bool shouldShowPrompt = restorePromptVisible && !compactMode;

    restoreOverlay.setVisible(shouldShowPrompt);

    if (shouldShowPrompt)
        restoreOverlay.toFront(false);
}


void PlayerGUI::setCompactMode(bool shouldBeCompact)
{
    if (compactMode == shouldBeCompact)
        return;

    compactMode = shouldBeCompact;

    if (compactMode && activeSidebarTab != SidebarTab::Playlist && activeSidebarTab != SidebarTab::Markers)
        activeSidebarTab = SidebarTab::Playlist;

    refreshButtonVisibility();
    updateRestorePromptVisibility();
    resized();
    repaint();
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

bool PlayerGUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& path : files)
    {
        const juce::File file(path);

        if (isSupportedAudioFile(file) || isSupportedLyricsFile(file) || isKnownUnsupportedAudioFile(file))
            return true;
    }

    return false;
}

void PlayerGUI::filesDropped(const juce::StringArray& files, int, int)
{
    juce::Array<juce::File> audioFiles;
    juce::StringArray unsupportedAudioNames;
    juce::File firstLyricsFile;

    for (const auto& path : files)
    {
        const juce::File file(path);

        if (isSupportedAudioFile(file))
            audioFiles.add(file);
        else if (isSupportedLyricsFile(file) && !firstLyricsFile.existsAsFile())
            firstLyricsFile = file;
        else if (isKnownUnsupportedAudioFile(file))
            unsupportedAudioNames.add(file.getFileName());
    }

    if (audioFiles.size() > 0)
        appendAudioFiles(audioFiles);

    if (unsupportedAudioNames.size() > 0)
        showUnsupportedAudioFormatMessage(unsupportedAudioNames);

    if (firstLyricsFile.existsAsFile())
        loadLyricsFromFile(firstLyricsFile);
}

void PlayerGUI::refreshButtonVisibility()
{
    startButton.setVisible(!compactMode);
    endButton.setVisible(!compactMode);
    restartButton.setVisible(!compactMode);

    setAButton.setVisible(!compactMode);
    setBButton.setVisible(!compactMode);
    abLoopButton.setVisible(!compactMode);
    addMarkerButton.setVisible(!compactMode);
    deleteMarkerButton.setVisible(!compactMode);

    saveQueueButton.setVisible(false);
    loadQueueButton.setVisible(false);
    removeTrackButton.setVisible(compactMode);
    clearQueueButton.setVisible(compactMode);
    moveUpButton.setVisible(false);
    moveDownButton.setVisible(false);
    repeatOneButton.setVisible(true);
    autoLyricsButton.setVisible(!compactMode);
    loadLyricsButton.setVisible(!compactMode);
    clearLyricsButton.setVisible(!compactMode);

    // These details now live in the sidebar tabs, especially Track Info.
    // Keeping this top row quiet makes the deck feel cleaner.
    queueStatusLabel.setVisible(false);
    lyricsStatusLabel.setVisible(false);
}

void PlayerGUI::resized()
{
    refreshButtonVisibility();
    updateRestorePromptVisibility();

    thumbnailArea = {};
    overviewArea = {};
    albumArtArea = {};
    transportDeckArea = {};
    markersCardArea = {};
    toolsCardArea = {};
    trackInfoArea = {};
    sidebarPanelArea = {};

    auto outer = getLocalBounds().reduced(compactMode ? 10 : 16);

    const int headerH = compactMode ? 40 : 52;
    outer.removeFromTop(headerH);
    outer.removeFromTop(compactMode ? 8 : 12);

    const int buttonH = compactMode ? 28 : 34;
    const int labelH = 24;

    auto sideArea = juce::Rectangle<int>();
    const bool showSide = outer.getWidth() >= 1120;

    if (showSide)
    {
        const int sideW = juce::jlimit(360, compactMode ? 360 : 430, outer.getWidth() / 4);
        sideArea = outer.removeFromRight(sideW);
        sidebarPanelArea = sideArea;
        outer.removeFromRight(compactMode ? 10 : 16);
    }

    auto mainArea = outer;

    if (compactMode)
    {
        auto deck = mainArea;

        auto nowPlayingRow = deck.removeFromTop(24);
        titleLabel.setBounds(nowPlayingRow);
        queueStatusLabel.setVisible(false);
        lyricsStatusLabel.setVisible(false);

        deck.removeFromTop(6);

        // Dual view now follows the same reading order as Pulse One/Two:
        // track title -> waveform -> time/progress -> controls.
        const int compactTransportH = juce::jlimit(86, 98, deck.getHeight() / 3);
        const int timelineH = 24;
        const int waveH = juce::jlimit(92, 126, deck.getHeight() - compactTransportH - timelineH - 20);

        thumbnailArea = deck.removeFromTop(waveH);
        albumArtArea = {};
        overviewArea = {};

        deck.removeFromTop(6);
        auto timelineRow = deck.removeFromTop(timelineH);
        durationLabel.setBounds(timelineRow.removeFromLeft(112));
        timelineRow.removeFromLeft(8);
        positionLabel.setBounds(timelineRow.removeFromLeft(134));
        timelineRow.removeFromLeft(8);
        positionSlider.setBounds(timelineRow.reduced(0, 4));

        deck.removeFromTop(6);
        transportDeckArea = deck.removeFromTop(compactTransportH);

        auto controls = transportDeckArea.reduced(14, 9);
        const int leftW = juce::jlimit(240, 310, controls.getWidth() / 4);
        const int rightW = juce::jlimit(290, 360, controls.getWidth() / 3);
        auto leftBlock = controls.removeFromLeft(leftW);
        auto rightBlock = controls.removeFromRight(rightW);
        auto centre = controls.reduced(16, 0);

        // Volume/Sound is kept as one clean group in the top-left of each compact deck.
        auto volumeLine = leftBlock.removeFromTop(30);
        volumeSlider.setBounds(volumeLine.removeFromLeft(leftW - 18).reduced(2, 6));
        leftBlock.removeFromTop(7);
        muteButton.setBounds(leftBlock.removeFromLeft(112).withHeight(30));

        auto tempoLine = rightBlock.removeFromTop(28);
        speedLabel.setBounds(tempoLine.removeFromLeft(96).withHeight(24));
        tempoLine.removeFromLeft(8);
        speedSlider.setBounds(tempoLine.reduced(0, 4));

        rightBlock.removeFromTop(8);
        auto modes = rightBlock.removeFromTop(40);
        const int modeGap = 8;
        const int modeW = (modes.getWidth() - modeGap * 2) / 3;
        loopButton.setBounds(modes.removeFromLeft(modeW).withHeight(36));
        modes.removeFromLeft(modeGap);
        shuffleButton.setBounds(modes.removeFromLeft(modeW).withHeight(36));
        modes.removeFromLeft(modeGap);
        repeatOneButton.setBounds(modes.removeFromLeft(modeW).withHeight(36));

        const int small = juce::jlimit(34, 42, centre.getWidth() / 9);
        const int big = juce::jlimit(56, 68, centre.getWidth() / 6);
        const int controlGap = 8;
        const int totalControlsW = small * 7 + big + controlGap * 7;
        int controlX = centre.getCentreX() - totalControlsW / 2;
        const int smallY = centre.getCentreY() - small / 2;
        const int bigY = centre.getCentreY() - big / 2;

        previousTrackButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        backwardButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        loadButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        playPauseButton.setBounds(controlX, bigY, big, big); controlX += big + controlGap;
        stopButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        forwardButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        nextTrackButton.setBounds(controlX, smallY, small, small);
    }
    else
    {
        auto deck = mainArea;

        auto nowPlayingRow = deck.removeFromTop(30);
        titleLabel.setBounds(nowPlayingRow);
        queueStatusLabel.setVisible(false);
        lyricsStatusLabel.setVisible(false);

        deck.removeFromTop(8);

        const int bottomCardsH = juce::jlimit(106, 126, deck.getHeight() / 5);
        const int transportH = 106;
        const int timelineH = 30;
        const int reserved = bottomCardsH + transportH + timelineH + 34;
        const int waveH = juce::jlimit(190, 360, deck.getHeight() - reserved);

        thumbnailArea = deck.removeFromTop(waveH);
        // Album art belongs to Track Info now, not over the waveform.
        // This keeps the waveform clean and makes the disc feel like metadata/identity.
        albumArtArea = {};
        deck.removeFromTop(10);

        // One clean waveform only. The progress slider below it now acts as the navigator.
        overviewArea = {};

        auto timelineRow = deck.removeFromTop(timelineH);
        durationLabel.setBounds(timelineRow.removeFromLeft(116));
        timelineRow.removeFromLeft(10);
        positionLabel.setBounds(timelineRow.removeFromLeft(142));
        timelineRow.removeFromLeft(10);
        positionSlider.setBounds(timelineRow.reduced(0, 5));

        deck.removeFromTop(10);
        transportDeckArea = deck.removeFromTop(transportH);

        auto controls = transportDeckArea.reduced(18, 12);

        auto leftBlock = controls.removeFromLeft(juce::jlimit(270, 335, controls.getWidth() / 4));
        auto rightBlock = controls.removeFromRight(juce::jlimit(280, 330, controls.getWidth() / 3));
        auto centre = controls.reduced(16, 0);

        auto volumeRow = leftBlock.removeFromTop(32);
        volumeSlider.setBounds(volumeRow.removeFromLeft(205).reduced(2, 5));
        leftBlock.removeFromTop(7);
        muteButton.setBounds(leftBlock.removeFromLeft(118).withHeight(36));

        auto tempoRow = rightBlock.removeFromTop(30);
        speedLabel.setBounds(tempoRow.removeFromLeft(102).withHeight(24));
        tempoRow.removeFromLeft(8);
        speedSlider.setBounds(tempoRow.reduced(0, 3));

        rightBlock.removeFromTop(10);
        auto modeBlock = rightBlock.removeFromTop(46);
        const int modeGap = 10;
        const int modeButtonW = (modeBlock.getWidth() - modeGap * 2) / 3;
        loopButton.setBounds(modeBlock.removeFromLeft(modeButtonW).withHeight(44));
        modeBlock.removeFromLeft(modeGap);
        shuffleButton.setBounds(modeBlock.removeFromLeft(modeButtonW).withHeight(44));
        modeBlock.removeFromLeft(modeGap);
        repeatOneButton.setBounds(modeBlock.removeFromLeft(modeButtonW).withHeight(44));

        const int small = juce::jlimit(36, 42, centre.getWidth() / 13);
        const int big = juce::jlimit(62, 72, centre.getWidth() / 8);
        const int controlGap = 7;
        const int totalControlsW = small * 9 + big + controlGap * 9;
        int controlX = centre.getCentreX() - totalControlsW / 2;
        const int smallY = centre.getCentreY() - small / 2;
        const int bigY = centre.getCentreY() - big / 2;

        previousTrackButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        loadButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        restartButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        backwardButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        playPauseButton.setBounds(controlX, bigY, big, big); controlX += big + controlGap;
        stopButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        forwardButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        nextTrackButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        startButton.setBounds(controlX, smallY, small, small); controlX += small + controlGap;
        endButton.setBounds(controlX, smallY, small, small);

        deck.removeFromTop(12);
        auto cards = deck;
        const int cardGap = 14;
        markersCardArea = cards.removeFromLeft((cards.getWidth() - cardGap) / 2);
        cards.removeFromLeft(cardGap);
        toolsCardArea = cards;

        auto markerInner = markersCardArea.reduced(18, 0).withTrimmedTop(46);
        auto markerTools = markerInner.removeFromTop(buttonH);
        const int markerButtonW = juce::jlimit(74, 96, markerTools.getWidth() / 6);
        setAButton.setBounds(markerTools.removeFromLeft(markerButtonW).withHeight(buttonH));
        markerTools.removeFromLeft(10);
        setBButton.setBounds(markerTools.removeFromLeft(markerButtonW).withHeight(buttonH));
        markerTools.removeFromLeft(10);
        abLoopButton.setBounds(markerTools.removeFromLeft(markerButtonW + 10).withHeight(buttonH));
        markerTools.removeFromLeft(16);
        addMarkerButton.setBounds(markerTools.removeFromLeft(markerButtonW).withHeight(buttonH));
        markerTools.removeFromLeft(10);
        deleteMarkerButton.setBounds(markerTools.removeFromLeft(markerButtonW).withHeight(buttonH));

        auto lyricInner = toolsCardArea.reduced(18, 0).withTrimmedTop(46);
        auto lyricTools = lyricInner.removeFromTop(buttonH);
        autoLyricsButton.setBounds(lyricTools.removeFromLeft(130).withHeight(buttonH));
        lyricTools.removeFromLeft(12);
        loadLyricsButton.setBounds(lyricTools.removeFromLeft(116).withHeight(buttonH));
        lyricTools.removeFromLeft(12);
        clearLyricsButton.setBounds(lyricTools.removeFromLeft(116).withHeight(buttonH));
    }

    if (showSide)
    {
        const int panelGap = compactMode ? 10 : 14;
        const int tabH = compactMode ? 30 : 34;

        playlistToggleButton.setVisible(true);
        markerToggleButton.setVisible(true);
        lyricsToggleButton.setVisible(!compactMode);
        trackInfoToggleButton.setVisible(!compactMode);

        if (compactMode && activeSidebarTab != SidebarTab::Playlist && activeSidebarTab != SidebarTab::Markers)
            activeSidebarTab = SidebarTab::Playlist;

        playlistToggleButton.setToggleState(activeSidebarTab == SidebarTab::Playlist, juce::dontSendNotification);
        lyricsToggleButton.setToggleState(activeSidebarTab == SidebarTab::Lyrics, juce::dontSendNotification);
        markerToggleButton.setToggleState(activeSidebarTab == SidebarTab::Markers, juce::dontSendNotification);
        trackInfoToggleButton.setToggleState(activeSidebarTab == SidebarTab::TrackInfo, juce::dontSendNotification);

        if (compactMode)
        {
            auto tabs = sideArea.removeFromTop(tabH).reduced(8, 0);
            const int tabGap = 8;
            const int tabW = (tabs.getWidth() - tabGap) / 2;
            playlistToggleButton.setBounds(tabs.removeFromLeft(tabW));
            tabs.removeFromLeft(tabGap);
            markerToggleButton.setBounds(tabs.removeFromLeft(tabW));
            sideArea.removeFromTop(panelGap);
        }
        else
        {
            sideArea.removeFromTop(42);
            auto tabs = sideArea.removeFromTop(tabH).reduced(8, 0);
            const int tabGap = 6;
            const int tabW = juce::jmax(54, (tabs.getWidth() - tabGap * 3) / 4);

            playlistToggleButton.setBounds(tabs.removeFromLeft(tabW));
            tabs.removeFromLeft(tabGap);
            lyricsToggleButton.setBounds(tabs.removeFromLeft(tabW));
            tabs.removeFromLeft(tabGap);
            markerToggleButton.setBounds(tabs.removeFromLeft(tabW));
            tabs.removeFromLeft(tabGap);
            trackInfoToggleButton.setBounds(tabs.removeFromLeft(tabW));
            sideArea.removeFromTop(panelGap);
        }

        const bool showPlaylist = compactMode ? activeSidebarTab != SidebarTab::Markers
            : activeSidebarTab == SidebarTab::Playlist;
        const bool showLyrics = !compactMode && activeSidebarTab == SidebarTab::Lyrics;
        const bool showMarkers = compactMode ? activeSidebarTab == SidebarTab::Markers
            : activeSidebarTab == SidebarTab::Markers;
        const bool showTrackInfo = !compactMode && activeSidebarTab == SidebarTab::TrackInfo;

        playlistBox.setVisible(showPlaylist);
        lyricsListBox.setVisible(showLyrics);
        markerListBox.setVisible(showMarkers);

        const bool showPlaylistTools = showPlaylist && !compactMode;
        loadQueueButton.setVisible(showPlaylistTools);
        saveQueueButton.setVisible(showPlaylistTools);
        removeTrackButton.setVisible(showPlaylist || (!compactMode && showPlaylistTools));
        clearQueueButton.setVisible(showPlaylist || (!compactMode && showPlaylistTools));
        moveUpButton.setVisible(showPlaylistTools);
        moveDownButton.setVisible(showPlaylistTools);

        addMarkerButton.setVisible(compactMode ? showMarkers : !compactMode);
        deleteMarkerButton.setVisible(compactMode ? showMarkers : !compactMode);
        trackInfoArea = {};

        if (compactMode)
        {
            auto compactPanel = sideArea.reduced(12);
            auto toolRow = compactPanel.removeFromBottom(buttonH);
            compactPanel.removeFromBottom(8);
            auto contentArea = compactPanel.withTrimmedTop(labelH);

            const int toolGap = 8;
            const int toolW = (toolRow.getWidth() - toolGap) / 2;

            if (showPlaylist)
            {
                playlistBox.setBounds(contentArea);
                removeTrackButton.setBounds(toolRow.removeFromLeft(toolW));
                toolRow.removeFromLeft(toolGap);
                clearQueueButton.setBounds(toolRow.removeFromLeft(toolW));
            }
            else
            {
                markerListBox.setBounds(contentArea);
                addMarkerButton.setBounds(toolRow.removeFromLeft(toolW));
                toolRow.removeFromLeft(toolGap);
                deleteMarkerButton.setBounds(toolRow.removeFromLeft(toolW));
            }
        }
        else if (showPlaylist)
        {
            auto playlistPanel = sideArea;
            auto reduced = playlistPanel.reduced(14);
            auto toolRow1 = reduced.removeFromBottom(buttonH);
            reduced.removeFromBottom(8);
            auto toolRow0 = reduced.removeFromBottom(buttonH);
            reduced.removeFromBottom(8);

            playlistBox.setBounds(reduced.withTrimmedTop(labelH));

            const int smallGap = 7;
            const int w0 = (toolRow0.getWidth() - smallGap * 2) / 3;
            removeTrackButton.setBounds(toolRow0.removeFromLeft(w0));
            toolRow0.removeFromLeft(smallGap);
            clearQueueButton.setBounds(toolRow0.removeFromLeft(w0));
            toolRow0.removeFromLeft(smallGap);
            moveUpButton.setBounds(toolRow0.removeFromLeft(w0));

            const int w1 = (toolRow1.getWidth() - smallGap * 2) / 3;
            moveDownButton.setBounds(toolRow1.removeFromLeft(w1));
            toolRow1.removeFromLeft(smallGap);
            saveQueueButton.setBounds(toolRow1.removeFromLeft(w1));
            toolRow1.removeFromLeft(smallGap);
            loadQueueButton.setBounds(toolRow1.removeFromLeft(w1));
        }
        else if (showMarkers)
        {
            markerListBox.setBounds(sideArea.reduced(14).withTrimmedTop(labelH));
        }
        else if (showLyrics)
        {
            lyricsListBox.setBounds(sideArea.reduced(14).withTrimmedTop(labelH));
        }
        else if (showTrackInfo)
        {
            trackInfoArea = sideArea.reduced(14).withTrimmedTop(labelH);

            const int discSize = juce::jlimit(104, 150, trackInfoArea.getWidth() / 2);
            albumArtArea.setBounds(trackInfoArea.getCentreX() - discSize / 2,
                trackInfoArea.getY() + 18,
                discSize,
                discSize);
        }
    }
    else
    {
        playlistToggleButton.setVisible(false);
        lyricsToggleButton.setVisible(false);
        markerToggleButton.setVisible(false);
        trackInfoToggleButton.setVisible(false);
        playlistBox.setVisible(false);
        markerListBox.setVisible(false);
        lyricsListBox.setVisible(false);
        loadQueueButton.setVisible(false);
        saveQueueButton.setVisible(false);
        removeTrackButton.setVisible(false);
        clearQueueButton.setVisible(false);
        moveUpButton.setVisible(false);
        moveDownButton.setVisible(false);
    }

    if (restorePromptVisible && !compactMode)
    {
        restoreOverlay.setBounds(getLocalBounds());
        restoreOverlay.toFront(false);
    }
}

void PlayerGUI::paint(juce::Graphics& g)
{
    const auto cardTop = juce::Colour::fromRGB(6, 7, 30);
    const auto cardBottom = juce::Colour::fromRGB(1, 2, 10);
    const auto panel = juce::Colour::fromRGB(14, 14, 42);
    const auto panelDark = juce::Colour::fromRGB(3, 4, 16);
    const auto accent = juce::Colour::fromRGB(31, 229, 255);
    const auto accentGreen = juce::Colour::fromRGB(92, 241, 218);
    const auto accentPink = juce::Colour::fromRGB(255, 49, 206);
    const auto accentViolet = juce::Colour::fromRGB(112, 58, 255);
    const auto border = juce::Colour::fromRGB(78, 71, 180);
    const auto softText = juce::Colours::white.withAlpha(0.72f);

    auto bounds = getLocalBounds();
    juce::ColourGradient backgroundGradient(cardTop, 0.0f, 0.0f, cardBottom, static_cast<float>(getWidth()), static_cast<float>(getHeight()), false);
    g.setGradientFill(backgroundGradient);
    g.fillAll();

    // Ambient neon washes inspired by the intro reference: subtle, not noisy.
    {
        auto b = getLocalBounds().toFloat();
        juce::ColourGradient cyanGlow(accent.withAlpha(0.16f), b.getWidth() * 0.18f, b.getHeight() * 0.18f,
            juce::Colours::transparentBlack, b.getWidth() * 0.62f, b.getHeight() * 0.48f, true);
        g.setGradientFill(cyanGlow);
        g.fillRect(getLocalBounds());

        juce::ColourGradient pinkGlow(accentPink.withAlpha(0.14f), b.getWidth() * 0.82f, b.getHeight() * 0.58f,
            juce::Colours::transparentBlack, b.getWidth() * 0.32f, b.getHeight() * 0.72f, true);
        g.setGradientFill(pinkGlow);
        g.fillRect(getLocalBounds());

        juce::ColourGradient violetGlow(accentViolet.withAlpha(0.12f), b.getWidth() * 0.48f, b.getHeight() * 0.05f,
            juce::Colours::transparentBlack, b.getWidth() * 0.50f, b.getHeight() * 0.72f, true);
        g.setGradientFill(violetGlow);
        g.fillRect(getLocalBounds());
    }

    auto fullCard = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(juce::Colours::black.withAlpha(0.18f));
    g.fillRoundedRectangle(fullCard.translated(0.0f, 2.0f), 14.0f);
    g.setColour(border.withAlpha(0.90f));
    g.drawRoundedRectangle(fullCard, 14.0f, 1.4f);

    auto headerArea = bounds.removeFromTop(compactMode ? 40 : 52);
    juce::ColourGradient headerGradient(juce::Colour::fromRGB(21, 18, 58), 0.0f, static_cast<float>(headerArea.getY()),
        juce::Colour::fromRGB(6, 7, 23), static_cast<float>(getWidth()), static_cast<float>(headerArea.getBottom()), false);
    g.setGradientFill(headerGradient);
    g.fillRect(headerArea);

    g.setColour(accent.withAlpha(0.28f));
    g.drawLine(20.0f, static_cast<float>(headerArea.getBottom()) - 1.0f,
        static_cast<float>(getWidth()) - 20.0f, static_cast<float>(headerArea.getBottom()) - 1.0f, 1.6f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(compactMode ? 19.0f : 24.0f, juce::Font::bold)));
    g.drawText(pulseTitle, headerArea.reduced(22, 0), juce::Justification::centredLeft);

    g.setColour(accent);
    g.setFont(juce::Font(juce::FontOptions(12.5f, juce::Font::bold)));
    g.drawText("SONICVIBE", headerArea.reduced(22, 0), juce::Justification::centredRight);

    auto drawSection = [&](juce::Rectangle<int> area, const juce::String& title, const juce::String& tag = {})
        {
            if (area.isEmpty())
                return;

            g.setColour(juce::Colours::black.withAlpha(0.24f));
            g.fillRoundedRectangle(area.toFloat().translated(0.0f, 2.0f), 12.0f);

            juce::ColourGradient grad(panel,
                static_cast<float>(area.getX()), static_cast<float>(area.getY()),
                panelDark,
                static_cast<float>(area.getRight()), static_cast<float>(area.getBottom()),
                false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(area.toFloat(), 12.0f);

            g.setColour(border.withAlpha(0.76f));
            g.drawRoundedRectangle(area.toFloat(), 12.0f, 1.15f);
            g.setColour(accentViolet.withAlpha(0.10f));
            g.drawRoundedRectangle(area.toFloat().reduced(1.5f), 11.0f, 0.8f);

            if (title.isNotEmpty() || tag.isNotEmpty())
            {
                auto titleBar = area.withHeight(34);
                juce::ColourGradient titleGrad(accentPink.withAlpha(0.18f),
                    static_cast<float>(titleBar.getX()), static_cast<float>(titleBar.getCentreY()),
                    juce::Colours::transparentBlack,
                    static_cast<float>(titleBar.getRight()), static_cast<float>(titleBar.getCentreY()),
                    false);
                g.setGradientFill(titleGrad);
                g.fillRoundedRectangle(titleBar.toFloat(), 12.0f);
                g.setColour(accent.withAlpha(0.25f));
                g.drawLine(static_cast<float>(titleBar.getX() + 12), static_cast<float>(titleBar.getBottom() - 1),
                    static_cast<float>(titleBar.getRight() - 12), static_cast<float>(titleBar.getBottom() - 1), 1.1f);

                g.setColour(juce::Colours::white.withAlpha(0.92f));
                g.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
                g.drawText(title, titleBar.reduced(14, 0), juce::Justification::centredLeft);

                if (tag.isNotEmpty())
                {
                    g.setColour(accent.withAlpha(0.92f));
                    g.setFont(juce::Font(juce::FontOptions(12.5f, juce::Font::bold)));
                    g.drawText(tag, titleBar.reduced(14, 0), juce::Justification::centredRight);
                }
            }
        };

    auto drawWaveformPanel = [&](juce::Rectangle<int> area, bool overview)
        {
            if (area.isEmpty())
                return;

            g.setColour(juce::Colours::black.withAlpha(0.26f));
            g.fillRoundedRectangle(area.toFloat().translated(0.0f, 2.0f), overview ? 8.0f : 12.0f);
            juce::ColourGradient wavePanelGrad(overview ? juce::Colour::fromRGB(7, 20, 31) : juce::Colour::fromRGB(10, 11, 36),
                static_cast<float>(area.getX()), static_cast<float>(area.getY()),
                overview ? juce::Colour::fromRGB(10, 35, 45) : juce::Colour::fromRGB(4, 4, 16),
                static_cast<float>(area.getRight()), static_cast<float>(area.getBottom()),
                false);
            g.setGradientFill(wavePanelGrad);
            g.fillRoundedRectangle(area.toFloat(), overview ? 8.0f : 12.0f);
            g.setColour(border.withAlpha(overview ? 0.52f : 0.86f));
            g.drawRoundedRectangle(area.toFloat(), overview ? 8.0f : 12.0f, overview ? 1.0f : 1.15f);

            if (!overview)
            {
                g.setColour(accent.withAlpha(0.84f));
                g.setFont(juce::Font(juce::FontOptions(12.5f, juce::Font::bold)));
                g.drawText("Waveform", area.reduced(14, 10).withHeight(18), juce::Justification::centredLeft);
            }

            if (thumbnailLoaded && thumbnail.getTotalLength() > 0.0)
            {
                auto waveArea = overview ? area.reduced(8, 5) : area.reduced(12, compactMode ? 24 : 34);
                if (!overview)
                {
                    g.setColour(accentPink.withAlpha(0.22f));
                    thumbnail.drawChannel(g, waveArea.translated(0, 1), 0.0, thumbnail.getTotalLength(), 0, 1.04f);
                    g.setColour(juce::Colour::fromRGB(39, 114, 255).withAlpha(0.34f));
                    thumbnail.drawChannel(g, waveArea.translated(0, -1), 0.0, thumbnail.getTotalLength(), 0, 1.02f);
                    g.setColour(accent.withAlpha(0.86f));
                    thumbnail.drawChannel(g, waveArea, 0.0, thumbnail.getTotalLength(), 0, 1.0f);
                    g.setColour(accentGreen.withAlpha(0.44f));
                    thumbnail.drawChannel(g, waveArea.reduced(0, 1), 0.0, thumbnail.getTotalLength(), 0, 0.92f);
                }
                else
                {
                    g.setColour(accent.withAlpha(0.30f));
                    thumbnail.drawChannel(g, waveArea, 0.0, thumbnail.getTotalLength(), 0, 0.56f);
                }

                const double audioLen = playerAudio.getLength();
                if (audioLen > 0.0)
                {
                    const double relativeX = juce::jlimit(0.0, 1.0, playerAudio.getPosition() / audioLen);
                    const int xPos = area.getX() + juce::roundToInt(area.getWidth() * relativeX);

                    g.setColour(accentPink.withAlpha(overview ? 0.86f : 1.0f));
                    g.drawLine(static_cast<float>(xPos), static_cast<float>(waveArea.getY()),
                        static_cast<float>(xPos), static_cast<float>(waveArea.getBottom()), overview ? 1.5f : 2.0f);
                }
            }
            else
            {
                g.setColour(softText.withAlpha(0.78f));
                g.setFont(juce::Font(juce::FontOptions(overview ? 12.0f : 15.0f)));
                g.drawText(overview ? "overview" : "No waveform loaded", area.reduced(0, overview ? 0 : 22), juce::Justification::centred);
            }
        };

    if (!transportDeckArea.isEmpty())
        drawSection(transportDeckArea, "", "");

    drawSection(markersCardArea, "Markers & Sections", "Mkr/Sec");
    drawSection(toolsCardArea, "Tools & Lyrics", autoLyricsEnabled ? "Auto On" : "Auto Off");

    if (!sidebarPanelArea.isEmpty() && !compactMode)
    {
        drawSection(sidebarPanelArea, "SIDEBAR", "");
    }

    auto drawSmallLabel = [&](const juce::String& text, juce::Rectangle<int> labelArea)
        {
            if (labelArea.isEmpty())
                return;

            g.setColour(accent.withAlpha(0.92f));
            g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
            g.drawText(text, labelArea, juce::Justification::centredLeft);
        };

    drawSmallLabel("Volume  " + juce::String(juce::roundToInt(volumeSlider.getValue() * 100.0)) + "%",
        volumeSlider.getBounds().withHeight(18).translated(0, -18));

    auto drawModeLabel = [&](const juce::Button& button, const juce::String& text)
        {
            if (!button.isVisible())
                return;
            const auto textState = button.getButtonText().trim();
            const bool active = textState.endsWithIgnoreCase("On") || textState.equalsIgnoreCase("Muted") || textState.equalsIgnoreCase("Pause");
            g.setColour(active ? (textState.equalsIgnoreCase("Muted") ? juce::Colour::fromRGB(255, 105, 115) : accent)
                : juce::Colours::white.withAlpha(0.70f));
            g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
            g.drawText(text, button.getBounds().translated(0, button.getHeight()).withHeight(16), juce::Justification::centred);
        };

    // Icon buttons are intentionally label-free here; tooltips explain each action on hover.

    auto drawListPanel = [&](const juce::String& title, juce::Rectangle<int> listBounds)
        {
            if (listBounds.isEmpty())
                return;

            const auto titleAccent = title.equalsIgnoreCase("Markers")
                ? juce::Colour::fromRGB(178, 120, 255)
                : accent;

            auto panelBounds = listBounds.expanded(12).withTrimmedTop(-28);
            const auto panelFloat = panelBounds.toFloat();

            g.setColour(juce::Colours::black.withAlpha(0.30f));
            g.fillRoundedRectangle(panelFloat.translated(0.0f, 2.0f), 11.0f);

            juce::ColourGradient panelGrad(juce::Colour::fromRGB(10, 11, 34).withAlpha(0.94f),
                panelFloat.getX(), panelFloat.getY(),
                juce::Colour::fromRGB(3, 4, 15).withAlpha(0.98f),
                panelFloat.getRight(), panelFloat.getBottom(), false);
            g.setGradientFill(panelGrad);
            g.fillRoundedRectangle(panelFloat, 11.0f);

            auto titleBar = panelBounds.withHeight(30);
            juce::ColourGradient titleGrad(titleAccent.withAlpha(0.16f),
                static_cast<float>(titleBar.getX()), static_cast<float>(titleBar.getCentreY()),
                juce::Colours::transparentBlack,
                static_cast<float>(titleBar.getRight()), static_cast<float>(titleBar.getCentreY()), false);
            g.setGradientFill(titleGrad);
            g.fillRoundedRectangle(titleBar.toFloat(), 11.0f);

            g.setColour(titleAccent.withAlpha(0.28f));
            g.drawLine(static_cast<float>(titleBar.getX() + 12), static_cast<float>(titleBar.getBottom() - 1),
                static_cast<float>(titleBar.getRight() - 12), static_cast<float>(titleBar.getBottom() - 1), 1.0f);

            g.setColour(titleAccent.withAlpha(0.96f));
            g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
            g.drawText(title, titleBar.reduced(13, 0), juce::Justification::centredLeft);

            auto inner = listBounds.toFloat();
            g.setColour(juce::Colour::fromRGB(2, 3, 12).withAlpha(0.72f));
            g.fillRoundedRectangle(inner, 8.0f);

            g.setColour(border.withAlpha(0.56f));
            g.drawRoundedRectangle(panelFloat, 11.0f, 1.0f);
            g.setColour(titleAccent.withAlpha(0.12f));
            g.drawRoundedRectangle(inner, 8.0f, 0.8f);
        };

    if (playlistBox.isVisible())
        drawListPanel("Playlist", playlistBox.getBounds());
    if (lyricsListBox.isVisible())
        drawListPanel("Lyrics", lyricsListBox.getBounds());
    if (markerListBox.isVisible())
        drawListPanel("Markers", markerListBox.getBounds());

    if (playlistBox.isVisible() && playlist.empty())
    {
        g.setColour(softText.withAlpha(0.42f));
        g.setFont(juce::Font(juce::FontOptions(12.5f)));
        g.drawText("Drop audio files here", playlistBox.getBounds().reduced(10), juce::Justification::centred);
    }

    if (lyricsListBox.isVisible() && lyrics.empty())
    {
        g.setColour(softText.withAlpha(0.42f));
        g.setFont(juce::Font(juce::FontOptions(12.5f)));
        g.drawText("Load a synced .lrc file", lyricsListBox.getBounds().reduced(10), juce::Justification::centred);
    }

    if (markerListBox.isVisible() && markers.empty())
    {
        g.setColour(softText.withAlpha(0.42f));
        g.setFont(juce::Font(juce::FontOptions(12.5f)));
        g.drawText("No cue markers yet", markerListBox.getBounds().reduced(10), juce::Justification::centred);
    }

    if (!trackInfoArea.isEmpty())
    {
        auto infoFrame = trackInfoArea.reduced(8);
        g.setColour(juce::Colours::black.withAlpha(0.18f));
        g.fillRoundedRectangle(infoFrame.toFloat(), 10.0f);
        g.setColour(border.withAlpha(0.44f));
        g.drawRoundedRectangle(infoFrame.toFloat(), 10.0f, 1.0f);

        auto content = infoFrame.reduced(14, 12);

        auto coverCard = content.removeFromTop(230);
        juce::ColourGradient coverGrad(juce::Colour::fromRGB(10, 18, 42).withAlpha(0.90f),
            static_cast<float>(coverCard.getX()), static_cast<float>(coverCard.getY()),
            juce::Colour::fromRGB(4, 5, 18).withAlpha(0.94f),
            static_cast<float>(coverCard.getRight()), static_cast<float>(coverCard.getBottom()), false);
        g.setGradientFill(coverGrad);
        g.fillRoundedRectangle(coverCard.toFloat(), 12.0f);
        g.setColour(accent.withAlpha(0.12f));
        g.drawRoundedRectangle(coverCard.toFloat(), 12.0f, 0.9f);

        if (!albumArtArea.isEmpty() && currentAudioFile.existsAsFile())
        {
            auto discLabelArea = albumArtArea.withY(albumArtArea.getBottom() + 8).withHeight(18);
            g.setColour(accent.withAlpha(0.85f));
            g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
            g.drawText(albumArtImage.isValid() ? "Track Cover" : "SonicVibe Disc", discLabelArea, juce::Justification::centred);
        }

        content.removeFromTop(10);

        auto drawInfoRow = [&](const juce::String& key, const juce::String& value, int height = 34)
            {
                if (content.getHeight() < height)
                    return;

                auto r = content.removeFromTop(height);
                auto strip = r.reduced(0, 2).toFloat();
                g.setColour(juce::Colour::fromRGB(18, 19, 44).withAlpha(0.34f));
                g.fillRoundedRectangle(strip, 8.0f);
                g.setColour(accent.withAlpha(0.10f));
                g.drawRoundedRectangle(strip, 8.0f, 0.8f);

                auto inner = r.reduced(10, 0);
                auto keyArea = inner.removeFromLeft(92);
                g.setColour(accent.withAlpha(0.86f));
                g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
                g.drawText(key, keyArea, juce::Justification::centredLeft);

                g.setColour(juce::Colours::white.withAlpha(0.84f));
                g.setFont(juce::Font(juce::FontOptions(12.0f)));
                g.drawFittedText(value, inner, juce::Justification::centredLeft, key == "Track" ? 2 : 1, 0.84f);
            };

        drawInfoRow("Track", currentAudioFile.existsAsFile() ? currentAudioFile.getFileNameWithoutExtension() : "No track loaded", 42);
        drawInfoRow("Length", formatTime(playerAudio.getLength()));
        drawInfoRow("Position", formatTime(playerAudio.getPosition()));
        drawInfoRow("Tempo", juce::String(speedRatio, 2) + "x");
        drawInfoRow("Playlist", juce::String(static_cast<int>(playlist.size())) + " tracks");
    }

    drawWaveformPanel(thumbnailArea, false);

    auto drawRotatingDisc = [&]()
        {
            if (albumArtArea.isEmpty() || !currentAudioFile.existsAsFile())
                return;

            auto disc = albumArtArea.toFloat();
            const auto centre = disc.getCentre();

            g.setColour(juce::Colours::black.withAlpha(0.32f));
            g.fillEllipse(disc.translated(0.0f, 3.0f));

            juce::Path discClip;
            discClip.addEllipse(disc);

            g.saveState();
            g.reduceClipRegion(discClip, juce::AffineTransform());
            g.addTransform(juce::AffineTransform::rotation(static_cast<float>(discRotationRadians), centre.x, centre.y));

            if (albumArtImage.isValid())
            {
                g.drawImageWithin(albumArtImage,
                    juce::roundToInt(disc.getX()),
                    juce::roundToInt(disc.getY()),
                    juce::roundToInt(disc.getWidth()),
                    juce::roundToInt(disc.getHeight()),
                    juce::RectanglePlacement::fillDestination);
            }
            else
            {
                g.setColour(juce::Colour::fromRGB(11, 18, 30));
                g.fillEllipse(disc);
                for (float ring = 0.18f; ring <= 0.82f; ring += 0.16f)
                {
                    auto r = disc.withSizeKeepingCentre(disc.getWidth() * ring, disc.getHeight() * ring);
                    g.setColour((ring < 0.45f ? accentPink : accent).withAlpha(0.24f));
                    g.drawEllipse(r, 1.0f);
                }
                g.setColour(accent.withAlpha(0.72f));
                g.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
                g.drawText("SV", disc.toNearestInt(), juce::Justification::centred);
            }

            g.restoreState();

            g.setColour(juce::Colours::black.withAlpha(0.56f));
            g.fillEllipse(disc.withSizeKeepingCentre(disc.getWidth() * 0.24f, disc.getHeight() * 0.24f));
            g.setColour(juce::Colour::fromRGB(110, 78, 188).withAlpha(0.16f));
            g.fillEllipse(disc.expanded(6.0f));
            g.setColour(accent.withAlpha(0.92f));
            g.drawEllipse(disc, 1.5f);
            g.setColour(juce::Colour::fromRGB(170, 124, 235).withAlpha(0.26f));
            g.drawEllipse(disc.reduced(7.0f), 1.0f);
        };

    drawRotatingDisc();
}



void PlayerGUI::mouseDown(const juce::MouseEvent& event)
{
    if (thumbnailArea.contains(event.getPosition()) && playerAudio.getLength() > 0.0)
    {
        seekFromWaveformX(event.position.x);
        return;
    }
}

void PlayerGUI::mouseDrag(const juce::MouseEvent& event)
{
    if (thumbnailArea.contains(event.getMouseDownPosition()) && playerAudio.getLength() > 0.0)
        seekFromWaveformX(event.position.x);
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &restoreOverlay.restoreButton)
    {
        restorePromptVisible = false;
        updateRestorePromptVisibility();
        loadSession();
        resized();
        repaint();
        return;
    }

    if (button == &restoreOverlay.startEmptyButton)
    {
        restorePromptVisible = false;
        updateRestorePromptVisibility();
        getSessionFile().deleteFile();
        clearQueue();
        resized();
        repaint();
        return;
    }

    if (button == &playlistToggleButton)
    {
        activeSidebarTab = SidebarTab::Playlist;
        playlistPanelOpen = true;
        markerPanelOpen = false;
        resized();
        repaint();
        return;
    }

    if (button == &lyricsToggleButton)
    {
        activeSidebarTab = SidebarTab::Lyrics;
        playlistPanelOpen = false;
        markerPanelOpen = false;
        resized();
        repaint();
        return;
    }

    if (button == &markerToggleButton)
    {
        activeSidebarTab = SidebarTab::Markers;
        playlistPanelOpen = false;
        markerPanelOpen = true;
        resized();
        repaint();
        return;
    }

    if (button == &trackInfoToggleButton)
    {
        activeSidebarTab = SidebarTab::TrackInfo;
        playlistPanelOpen = false;
        markerPanelOpen = false;
        resized();
        repaint();
        return;
    }

    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select audio files...", juce::File{}, "*.mp3;*.wav;*.flac;*.ogg;*.aiff;*.aif");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles |
            juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                appendAudioFiles(fc.getResults());
            });

        return;
    }

    if (button == &loadQueueButton)
    {
        loadQueueFromFile();
        return;
    }

    if (button == &saveQueueButton)
    {
        saveQueueToFile();
        return;
    }

    if (button == &removeTrackButton)
    {
        removeTrackAt(playlistBox.getSelectedRow());
        return;
    }

    if (button == &clearQueueButton)
    {
        clearQueue();
        return;
    }

    if (button == &moveUpButton)
    {
        moveTrack(playlistBox.getSelectedRow(), -1);
        return;
    }

    if (button == &moveDownButton)
    {
        moveTrack(playlistBox.getSelectedRow(), 1);
        return;
    }

    if (button == &autoLyricsButton)
    {
        autoLyricsEnabled = !autoLyricsEnabled;
        autoLyricsButton.setButtonText(autoLyricsEnabled ? "Auto Lyrics On" : "Auto Lyrics Off");

        if (autoLyricsEnabled && currentAudioFile.existsAsFile())
            tryAutoLoadLyricsForTrack(currentAudioFile);
        else
            updateLyricsStatusLabel();

        return;
    }

    if (button == &loadLyricsButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Load synced lyrics...", juce::File{}, "*.lrc;*.txt");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                const auto file = fc.getResult();

                if (file.existsAsFile())
                    loadLyricsFromFile(file);
            });
        return;
    }

    if (button == &clearLyricsButton)
    {
        clearLyrics();
        return;
    }

    if (button == &previousTrackButton)
    {
        loadRelativeTrack(-1, isPlaying);
        return;
    }

    if (button == &nextTrackButton)
    {
        loadRelativeTrack(1, isPlaying);
        return;
    }

    if (button == &restartButton)
    {
        playerAudio.stop();
        setSyncedPlaybackPosition(0.0);
        playerAudio.start();
        markPlaybackStarted();
        isPlaying = true;
        playPauseButton.setButtonText("Pause");
        return;
    }

    if (button == &stopButton)
    {
        playerAudio.stop();
        setSyncedPlaybackPosition(0.0);
        isPlaying = false;
        playPauseButton.setButtonText("Play");
        return;
    }

    if (button == &playPauseButton)
    {
        if (!isPlaying)
        {
            if (playlist.empty() || playerAudio.getLength() <= 0.0)
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "SonicVibe", "Load an audio file first.");
                return;
            }

            playerAudio.start();
            markPlaybackStarted();
            isPlaying = true;
            playPauseButton.setButtonText("Pause");
        }
        else
        {
            markPlaybackPaused();
            playerAudio.stop();
            isPlaying = false;
            playPauseButton.setButtonText("Play");
        }

        return;
    }

    if (button == &startButton)
    {
        setSyncedPlaybackPosition(0.0);

        if (isPlaying)
            playerAudio.start();

        return;
    }

    if (button == &endButton)
    {
        const double len = playerAudio.getLength();

        if (len > 0.0)
        {
            setSyncedPlaybackPosition(len);
            markPlaybackPaused();
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
            previousGain = static_cast<float>(volumeSlider.getValue());
            playerAudio.setGain(0.0f);
            muteButton.setButtonText("Muted");
            muteButton.setToggleState(true, juce::dontSendNotification);
            isMuted = true;
        }
        else
        {
            playerAudio.setGain(previousGain);
            muteButton.setButtonText("Sound");
            muteButton.setToggleState(false, juce::dontSendNotification);
            isMuted = false;
        }

        return;
    }

    if (button == &loopButton)
    {
        isLooping = !isLooping;
        playerAudio.setLooping(isLooping);
        loopButton.setButtonText(isLooping ? "Loop On" : "Loop Off");
        loopButton.setToggleState(isLooping, juce::dontSendNotification);
        return;
    }

    if (button == &forwardButton)
    {
        setSyncedPlaybackPosition(juce::jmin(playerAudio.getPosition() + 10.0, playerAudio.getLength()));
        return;
    }

    if (button == &backwardButton)
    {
        setSyncedPlaybackPosition(juce::jmax(0.0, playerAudio.getPosition() - 10.0));
        return;
    }

    if (button == &setAButton)
    {
        if (playerAudio.getLength() <= 0.0)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "A-B Loop", "Load an audio file first.");
            return;
        }

        pointA = playerAudio.getPosition();

        if (pointB > pointA)
            playerAudio.setLoopPoints(pointA, pointB);

        setAButton.setButtonText("A: " + formatTime(pointA));
        return;
    }

    if (button == &setBButton)
    {
        if (playerAudio.getLength() <= 0.0)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "A-B Loop", "Load an audio file first.");
            return;
        }

        pointB = playerAudio.getPosition();

        if (pointB <= pointA)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "A-B Loop", "Point B must be after Point A.");
            return;
        }

        playerAudio.setLoopPoints(pointA, pointB);
        setBButton.setButtonText("B: " + formatTime(pointB));
        return;
    }

    if (button == &abLoopButton)
    {
        if (playerAudio.getLength() <= 0.0)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "A-B Loop", "Load an audio file first.");
            return;
        }

        if (pointB <= pointA)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "A-B Loop", "Set point A first, then set point B after it.");
            return;
        }

        abLoopEnabled = !abLoopEnabled;
        playerAudio.setLoopPoints(pointA, pointB);
        playerAudio.enableABLoop(abLoopEnabled);

        if (abLoopEnabled)
        {
            setSyncedPlaybackPosition(pointA);

            if (isPlaying)
                playerAudio.start();
        }

        abLoopButton.setButtonText(abLoopEnabled ? "A-B On" : "A-B Off");
        return;
    }

    if (button == &shuffleButton)
    {
        isShuffle = !isShuffle;
        shuffleButton.setButtonText(isShuffle ? "Shuffle On" : "Shuffle Off");
        shuffleButton.setToggleState(isShuffle, juce::dontSendNotification);
        return;
    }

    if (button == &repeatOneButton)
    {
        cycleRepeatMode();
        repaint();
        return;
    }

    if (button == &addMarkerButton)
    {
        if (playerAudio.getLength() <= 0.0)
            return;

        markers.push_back({ static_cast<int>(markers.size()) + 1, playerAudio.getPosition() });

        std::sort(markers.begin(), markers.end(), [](const auto& a, const auto& b)
            {
                return a.second < b.second;
            });

        markerListBox.updateContent();
        repaint();
        return;
    }

    if (button == &deleteMarkerButton)
    {
        const int selectedRow = markerListBox.getSelectedRow();

        if (selectedRow >= 0 && selectedRow < static_cast<int>(markers.size()))
        {
            markers.erase(markers.begin() + selectedRow);
            markerListBox.updateContent();
            repaint();
        }

        return;
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        playerAudio.setGain(static_cast<float>(slider->getValue()));
        return;
    }

    if (slider == &positionSlider)
    {
        const double len = playerAudio.getLength();

        if (len > 0.0)
        {
            setSyncedPlaybackPosition(positionSlider.getValue() * len);
            updateCurrentLyric();
        }

        return;
    }

    if (slider == &speedSlider)
    {
        applyTempoFromSlider();
        return;
    }
}



void PlayerGUI::seekFromWaveformX(float mouseX)
{
    const double len = playerAudio.getLength();
    if (len <= 0.0 || thumbnailArea.isEmpty())
        return;

    auto waveArea = thumbnailArea.reduced(12, compactMode ? 24 : 34);
    if (waveArea.getWidth() <= 0)
        return;

    const double ratio = juce::jlimit(0.0, 1.0,
        (static_cast<double>(mouseX) - static_cast<double>(waveArea.getX())) / static_cast<double>(waveArea.getWidth()));
    const double newPosition = ratio * len;

    setSyncedPlaybackPosition(newPosition);
    positionSlider.setValue(ratio, juce::dontSendNotification);
    positionLabel.setText(formatTime(newPosition) + " / " + formatTime(len), juce::dontSendNotification);
    updateCurrentLyric();

    if (isPlaying)
        playerAudio.start();

    repaint();
}

void PlayerGUI::loadAlbumArtForTrack(const juce::File& audioFile)
{
    albumArtImage = juce::Image{};
    discRotationRadians = 0.0;

    if (!audioFile.existsAsFile())
        return;

    const auto dir = audioFile.getParentDirectory();
    const auto base = audioFile.getFileNameWithoutExtension();

    juce::Array<juce::File> candidates;
    for (const auto& ext : { ".jpg", ".jpeg", ".png", ".gif" })
        candidates.add(dir.getChildFile(base + ext));

    for (const auto& name : { "cover", "Cover", "folder", "Folder", "album", "Album", "artwork", "Artwork" })
    {
        for (const auto& ext : { ".jpg", ".jpeg", ".png", ".gif" })
            candidates.add(dir.getChildFile(juce::String(name) + ext));
    }

    for (const auto& candidate : candidates)
    {
        if (!candidate.existsAsFile())
            continue;

        auto image = juce::ImageFileFormat::loadFrom(candidate);
        if (image.isValid())
        {
            albumArtImage = image;
            return;
        }
    }

    // MP3 embedded artwork: read ID3v2 APIC frames directly, so the disc can show
    // covers that Windows Media Player already detects inside the file.
    auto embeddedImage = tryLoadID3v2AlbumArt(audioFile);
    if (embeddedImage.isValid())
        albumArtImage = embeddedImage;
}

void PlayerGUI::clearAlbumArt()
{
    albumArtImage = juce::Image{};
    discRotationRadians = 0.0;
}



void PlayerGUI::applyTempoFromSlider()
{
    const double newRatio = juce::jlimit(0.50, 2.00, speedSlider.getValue());

    if (std::abs(newRatio - speedRatio) < 0.0001)
    {
        speedLabel.setText("Tempo: " + juce::String(speedRatio, 2) + "x", juce::dontSendNotification);
        return;
    }

    // Anchor the visual timeline/lyrics at the real audio position before changing tempo.
    playbackAnchorPosition = playerAudio.getPosition();
    playbackAnchorTimeMs = juce::Time::getMillisecondCounterHiRes();

    speedRatio = newRatio;
    playerAudio.setSpeed(speedRatio);

    speedLabel.setText("Tempo: " + juce::String(speedRatio, 2) + "x", juce::dontSendNotification);
    updateCurrentLyric();
}

double PlayerGUI::getEffectivePlaybackPosition() const
{
    const double length = playerAudio.getLength();

    if (!isPlaying)
        return length > 0.0 ? juce::jlimit(0.0, length, playbackAnchorPosition)
        : juce::jmax(0.0, playbackAnchorPosition);

    const double rawAudioPos = playerAudio.getPosition();

    // At normal speed, the transport position is the source of truth. This keeps
    // looped playback, the waveform cursor, the slider, and lyrics perfectly tied.
    if (std::abs(speedRatio - 1.0) < 0.001)
        return length > 0.0 ? juce::jlimit(0.0, length, rawAudioPos)
        : juce::jmax(0.0, rawAudioPos);

    // If the audio transport wrapped because Loop is active, avoid clamping the UI at the end.
    if (length > 0.0 && rawAudioPos + 0.75 < playbackAnchorPosition)
        return juce::jlimit(0.0, length, rawAudioPos);

    const double nowMs = juce::Time::getMillisecondCounterHiRes();
    const double elapsedSeconds = juce::jmax(0.0, (nowMs - playbackAnchorTimeMs) / 1000.0);
    const double calculatedPosition = playbackAnchorPosition + elapsedSeconds * speedRatio;

    return length > 0.0 ? juce::jlimit(0.0, length, calculatedPosition)
        : juce::jmax(0.0, calculatedPosition);
}

void PlayerGUI::setSyncedPlaybackPosition(double seconds)
{
    const double length = playerAudio.getLength();
    const double clampedSeconds = length > 0.0 ? juce::jlimit(0.0, length, seconds)
        : juce::jmax(0.0, seconds);

    playerAudio.setPosition(clampedSeconds);
    playbackAnchorPosition = clampedSeconds;
    playbackAnchorTimeMs = juce::Time::getMillisecondCounterHiRes();
}

void PlayerGUI::markPlaybackStarted()
{
    playbackAnchorPosition = playerAudio.getPosition();
    playbackAnchorTimeMs = juce::Time::getMillisecondCounterHiRes();
}

void PlayerGUI::markPlaybackPaused()
{
    const double effectivePosition = getEffectivePlaybackPosition();
    playerAudio.setPosition(effectivePosition);
    playbackAnchorPosition = effectivePosition;
    playbackAnchorTimeMs = juce::Time::getMillisecondCounterHiRes();
}

bool PlayerGUI::keyPressed(const juce::KeyPress& key)
{
    const int code = key.getKeyCode();

    if (code == juce::KeyPress::spaceKey)
    {
        playPauseButton.triggerClick();
        return true;
    }

    if (code == juce::KeyPress::returnKey)
    {
        returnKeyPressed(playlistBox.getSelectedRow());
        return true;
    }

    if (code == juce::KeyPress::deleteKey || code == juce::KeyPress::backspaceKey)
    {
        removeTrackAt(playlistBox.getSelectedRow());
        return true;
    }

    if (code == juce::KeyPress::leftKey)
    {
        backwardButton.triggerClick();
        return true;
    }

    if (code == juce::KeyPress::rightKey)
    {
        forwardButton.triggerClick();
        return true;
    }

    if (code == juce::KeyPress::upKey || code == juce::KeyPress::downKey)
    {
        if (lyricsListBox.hasKeyboardFocus(true))
            return false;

        const int direction = (code == juce::KeyPress::upKey) ? -1 : 1;

        if (key.getModifiers().isCtrlDown())
        {
            moveTrack(playlistBox.getSelectedRow(), direction);
            return true;
        }

        if (!playlist.empty())
        {
            const int last = static_cast<int>(playlist.size()) - 1;
            const int current = playlistBox.getSelectedRow() >= 0 ? playlistBox.getSelectedRow() : 0;
            playlistBox.selectRow(juce::jlimit(0, last, current + direction));
            updateQueueStatusLabel();
            return true;
        }
    }

    return false;
}

int PlayerGUI::getNumRows()
{
    return static_cast<int>(playlist.size());
}

void PlayerGUI::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    const bool isValid = row >= 0 && row < static_cast<int>(playlist.size());
    const bool isCurrent = isValid && playlist[static_cast<size_t>(row)] == currentAudioFile;

    const auto base = juce::Colour::fromRGB(4, 5, 16);
    const auto selected = juce::Colour::fromRGB(30, 28, 70);
    const auto currentA = juce::Colour::fromRGB(14, 100, 134);
    const auto currentB = juce::Colour::fromRGB(62, 48, 148);
    const auto accent = juce::Colour::fromRGB(31, 229, 255);
    const auto accentPink = juce::Colour::fromRGB(255, 49, 206);
    const auto accentViolet = juce::Colour::fromRGB(112, 58, 255);

    g.fillAll(base);

    auto rowArea = juce::Rectangle<int>(4, 3, width - 8, height - 6).toFloat();

    if (isCurrent)
    {
        juce::ColourGradient rowGrad(currentA.withAlpha(0.92f), rowArea.getX(), rowArea.getCentreY(),
            currentB.withAlpha(0.82f), rowArea.getRight(), rowArea.getCentreY(), false);
        g.setGradientFill(rowGrad);
        g.fillRoundedRectangle(rowArea, 7.0f);

        g.setColour(accent.withAlpha(0.42f));
        g.drawRoundedRectangle(rowArea, 7.0f, 1.1f);
        g.setColour(accentPink.withAlpha(0.14f));
        g.drawLine(rowArea.getX() + 8.0f, rowArea.getBottom() - 1.0f, rowArea.getRight() - 8.0f, rowArea.getBottom() - 1.0f, 1.0f);
    }
    else if (rowIsSelected)
    {
        g.setColour(selected.withAlpha(0.78f));
        g.fillRoundedRectangle(rowArea, 7.0f);
        g.setColour(accentPink.withAlpha(0.22f));
        g.drawRoundedRectangle(rowArea, 7.0f, 0.9f);
    }

    if (isValid)
    {
        const auto numberText = juce::String(row + 1) + ".";
        const auto fileName = playlist[static_cast<size_t>(row)].getFileNameWithoutExtension();

        g.setFont(juce::Font(juce::FontOptions(12.2f, juce::Font::bold)));
        g.setColour(isCurrent ? accent : juce::Colours::white.withAlpha(0.52f));
        g.drawText(numberText, 12, 0, 34, height, juce::Justification::centredLeft);

        g.setFont(juce::Font(juce::FontOptions(12.7f, isCurrent ? juce::Font::bold : juce::Font::plain)));
        g.setColour(juce::Colours::white.withAlpha(isCurrent ? 0.96f : 0.82f));
        g.drawText(fileName, 48, 0, width - 58, height, juce::Justification::centredLeft);
    }
}

void PlayerGUI::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    loadTrackAt(row, false);
}

void PlayerGUI::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    loadTrackAt(row, true);
}

void PlayerGUI::deleteKeyPressed(int lastRowSelected)
{
    removeTrackAt(lastRowSelected);
}

void PlayerGUI::returnKeyPressed(int lastRowSelected)
{
    if (lastRowSelected < 0 || lastRowSelected >= static_cast<int>(playlist.size()))
        return;

    const auto& selectedFile = playlist[static_cast<size_t>(lastRowSelected)];

    if (selectedFile == currentAudioFile)
    {
        if (!isPlaying)
        {
            playerAudio.start();
            markPlaybackStarted();
            isPlaying = true;
            playPauseButton.setButtonText("Pause");
            repaint();
        }

        return;
    }

    loadTrackAt(lastRowSelected, true);
}

void PlayerGUI::timerCallback()
{
    if (std::abs(speedSlider.getValue() - speedRatio) > 0.0001)
        applyTempoFromSlider();

    const double len = playerAudio.getLength();
    const double rawAudioPos = playerAudio.getPosition();

    if (isPlaying && len > 0.0)
    {
        // Keep the UI clock tied to the real transport when normal speed is used.
        // This also fixes loop wrap: after the audio loops back to the start, the
        // waveform cursor, position slider, and lyrics must follow it immediately.
        if (std::abs(speedRatio - 1.0) < 0.001 || rawAudioPos + 0.75 < playbackAnchorPosition)
        {
            playbackAnchorPosition = rawAudioPos;
            playbackAnchorTimeMs = juce::Time::getMillisecondCounterHiRes();
        }

        discRotationRadians += 0.035 * juce::jlimit(0.5, 2.0, speedRatio);
        if (discRotationRadians > juce::MathConstants<double>::twoPi)
            discRotationRadians -= juce::MathConstants<double>::twoPi;
    }

    const double pos = getEffectivePlaybackPosition();

    positionSlider.setValue(len > 0.0 ? pos / len : 0.0, juce::dontSendNotification);
    positionLabel.setText(formatTime(pos) + " / " + formatTime(len), juce::dontSendNotification);

    if (!isLooping && !abLoopEnabled && isPlaying && !playlist.empty() && len > 0.0 && pos >= len - 0.2)
        handleTrackFinished();

    if (abLoopEnabled)
        playerAudio.checkAndLoopSegment();

    updateCurrentLyric();
    updateLyricsScrollAnimation();

    if (thumbnailLoaded || currentAudioFile.existsAsFile())
        repaint();
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

bool PlayerGUI::isSupportedAudioFile(const juce::File& file) const
{
    const auto ext = file.getFileExtension().toLowerCase();

    return file.existsAsFile()
        && (ext == ".mp3"
            || ext == ".wav"
            || ext == ".flac"
            || ext == ".ogg"
            || ext == ".aiff"
            || ext == ".aif");
}

bool PlayerGUI::isKnownUnsupportedAudioFile(const juce::File& file) const
{
    const auto ext = file.getFileExtension().toLowerCase();

    return file.existsAsFile()
        && (ext == ".m4a"
            || ext == ".ma4"
            || ext == ".aac"
            || ext == ".mp4"
            || ext == ".wma");
}

void PlayerGUI::showUnsupportedAudioFormatMessage(const juce::StringArray& fileNames) const
{
    juce::String message;

    if (fileNames.size() == 1)
        message = "This audio format is not supported yet:\n" + fileNames[0];
    else
        message = "These audio formats are not supported yet:\n" + fileNames.joinIntoString("\n");

    message += "\n\nSonicVibe currently supports: MP3, WAV, FLAC, OGG, AIFF/AIF.";
    message += "\nM4A/AAC/MP4 audio will need an FFmpeg-based decoder or converter later.";

    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "Unsupported audio format",
        message);
}

bool PlayerGUI::isSupportedLyricsFile(const juce::File& file) const
{
    const auto ext = file.getFileExtension().toLowerCase();
    return file.existsAsFile() && (ext == ".lrc" || ext == ".txt");
}

bool PlayerGUI::appendAudioFile(const juce::File& file)
{
    if (!isSupportedAudioFile(file))
        return false;

    playlist.push_back(file);
    return true;
}

void PlayerGUI::appendAudioFiles(const juce::Array<juce::File>& files)
{
    const bool wasEmpty = playlist.empty();
    int addedCount = 0;
    juce::StringArray unsupportedAudioNames;

    for (const auto& file : files)
    {
        if (appendAudioFile(file))
            ++addedCount;
        else if (isKnownUnsupportedAudioFile(file))
            unsupportedAudioNames.add(file.getFileName());
    }

    playlistBox.updateContent();
    updateQueueStatusLabel();

    if (unsupportedAudioNames.size() > 0)
        showUnsupportedAudioFormatMessage(unsupportedAudioNames);

    if (addedCount > 0 && wasEmpty)
        loadTrackAt(0, false);
    else if (addedCount > 0)
        repaint();
}

void PlayerGUI::loadTrackAt(int row, bool shouldStartPlayback)
{
    if (row < 0 || row >= static_cast<int>(playlist.size()))
        return;

    const auto& file = playlist[static_cast<size_t>(row)];
    const bool isSameTrackAsBefore = currentAudioFile == file;

    if (!playerAudio.loadFile(file))
    {
        juce::StringArray failedFiles;
        failedFiles.add(file.getFileName());
        showUnsupportedAudioFormatMessage(failedFiles);
        return;
    }

    thumbnail.setSource(new juce::FileInputSource(file));
    thumbnailLoaded = true;
    loadAlbumArtForTrack(file);

    titleLabel.setText("Name: " + file.getFileNameWithoutExtension(), juce::dontSendNotification);
    durationLabel.setText("Length: " + playerAudio.getFormattedLength(), juce::dontSendNotification);
    currentAudioFile = file;
    playbackAnchorPosition = 0.0;
    playbackAnchorTimeMs = juce::Time::getMillisecondCounterHiRes();

    playlistBox.selectRow(row);
    updateQueueStatusLabel();
    resetLoopState();
    markers.clear();
    markerListBox.updateContent();

    if (!isSameTrackAsBefore)
        tryAutoLoadLyricsForTrack(file);

    playerAudio.setGain(isMuted ? 0.0f : static_cast<float>(volumeSlider.getValue()));

    if (shouldStartPlayback)
    {
        playerAudio.start();
        markPlaybackStarted();
        isPlaying = true;
        playPauseButton.setButtonText("Pause");
    }
    else
    {
        isPlaying = false;
        playPauseButton.setButtonText("Play");
    }

    repaint();
}

void PlayerGUI::loadRelativeTrack(int offset, bool shouldStartPlayback)
{
    if (playlist.empty())
        return;

    const int current = playlistBox.getSelectedRow() >= 0 ? playlistBox.getSelectedRow() : 0;
    const int last = static_cast<int>(playlist.size()) - 1;
    const int next = juce::jlimit(0, last, current + offset);

    loadTrackAt(next, shouldStartPlayback);
}


void PlayerGUI::moveTrack(int fromRow, int direction)
{
    if (fromRow < 0 || fromRow >= static_cast<int>(playlist.size()))
        return;

    const int toRow = fromRow + direction;

    if (toRow < 0 || toRow >= static_cast<int>(playlist.size()))
        return;

    std::swap(playlist[static_cast<size_t>(fromRow)], playlist[static_cast<size_t>(toRow)]);
    playlistBox.updateContent();
    playlistBox.selectRow(toRow);
    updateQueueStatusLabel();
}

void PlayerGUI::updateQueueStatusLabel()
{
    const int total = static_cast<int>(playlist.size());
    const int selected = playlistBox.getSelectedRow();

    if (total <= 0)
    {
        queueStatusLabel.setText("Playlist: 0 tracks", juce::dontSendNotification);
        return;
    }

    const int current = selected >= 0 ? selected + 1 : 0;
    queueStatusLabel.setText("Playlist: " + juce::String(total) + " tracks" +
        (current > 0 ? "  •  Selected " + juce::String(current) + "/" + juce::String(total) : juce::String()),
        juce::dontSendNotification);
}

void PlayerGUI::updateLyricsStatusLabel()
{
    const juce::String autoState = autoLyricsEnabled ? "Auto On" : "Auto Off";

    if (lyrics.empty())
    {
        if (autoLyricsEnabled && currentAudioFile.existsAsFile())
        {
            const auto matchingLyrics = findMatchingLyricsFileForTrack(currentAudioFile);

            if (!matchingLyrics.existsAsFile())
            {
                lyricsStatusLabel.setText("Lyrics: " + autoState + " • no matching .lrc found", juce::dontSendNotification);
                return;
            }
        }

        lyricsStatusLabel.setText("Lyrics: " + autoState + " • none", juce::dontSendNotification);
        return;
    }

    const auto fileName = currentLyricsFile.existsAsFile() ? currentLyricsFile.getFileName() : juce::String("loaded");

    lyricsStatusLabel.setText("Lyrics: " + autoState + " • " + juce::String(lyrics.size()) + " lines  •  " + fileName,
        juce::dontSendNotification);
}

void PlayerGUI::setCurrentLyricIndex(int newIndex)
{
    if (newIndex == currentLyricIndex)
        return;

    currentLyricIndex = newIndex;
    lyricsListBox.updateContent();

    if (currentLyricIndex >= 0)
    {
        lyricsListBox.selectRow(currentLyricIndex, false, true);
        centerLyricsOnCurrentLine();
    }

    repaint();
}

void PlayerGUI::centerLyricsOnCurrentLine()
{
    if (currentLyricIndex < 0 || lyrics.empty())
        return;

    lyricsTargetScrollY = calculateCenteredLyricsScrollY();

    if (!lyricsScrollInitialised)
    {
        lyricsSmoothScrollY = static_cast<double>(lyricsTargetScrollY);
        lyricsScrollVelocityY = 0.0;
        lyricsScrollInitialised = true;

        if (auto* viewport = lyricsListBox.getViewport())
            viewport->setViewPosition(0, lyricsTargetScrollY);

        lyricsScrollAnimating = false;
        return;
    }

    // Animate only after the active lyric changes, then release the viewport.
    // This keeps the Apple/Spotify-like motion without fighting the user when
    // they manually scroll the lyrics list.
    lyricsScrollAnimating = true;
}

int PlayerGUI::calculateCenteredLyricsScrollY() const
{
    if (currentLyricIndex < 0 || lyrics.empty())
        return 0;

    if (auto* viewport = lyricsListBox.getViewport())
    {
        const int rowH = lyricsListBox.getRowHeight();
        const int totalRows = static_cast<int>(lyrics.size());
        const int targetY = currentLyricIndex * rowH - viewport->getViewHeight() / 2 + rowH / 2;
        const int maxY = juce::jmax(0, totalRows * rowH - viewport->getViewHeight());
        return juce::jlimit(0, maxY, targetY);
    }

    return 0;
}

void PlayerGUI::updateLyricsScrollAnimation()
{
    if (!lyricsScrollInitialised || !lyricsScrollAnimating || lyrics.empty())
        return;

    auto* viewport = lyricsListBox.getViewport();
    if (viewport == nullptr)
        return;

    const double distance = static_cast<double>(lyricsTargetScrollY) - lyricsSmoothScrollY;

    // Soft spring-damper motion: smoother than jumping directly to the new line,
    // but it stops quickly and releases the list so the user can scroll manually.
    lyricsScrollVelocityY = lyricsScrollVelocityY * 0.74 + distance * 0.055;
    lyricsSmoothScrollY += lyricsScrollVelocityY;

    if (std::abs(distance) < 0.80 && std::abs(lyricsScrollVelocityY) < 0.45)
    {
        lyricsSmoothScrollY = static_cast<double>(lyricsTargetScrollY);
        lyricsScrollVelocityY = 0.0;
        lyricsScrollAnimating = false;
    }

    viewport->setViewPosition(0, juce::roundToInt(lyricsSmoothScrollY));
}

void PlayerGUI::seekToLyricLine(int row)
{
    if (row < 0 || row >= static_cast<int>(lyrics.size()))
        return;

    const auto& line = lyrics[static_cast<size_t>(row)];

    if (line.timeSeconds < 0.0)
        return;

    const double length = playerAudio.getLength();
    const double targetPosition = juce::jlimit(0.0, length > 0.0 ? length : line.timeSeconds, line.timeSeconds);

    setSyncedPlaybackPosition(targetPosition);

    if (isPlaying)
        playerAudio.start();

    setCurrentLyricIndex(row);
    positionSlider.setValue(length > 0.0 ? targetPosition / length : 0.0, juce::dontSendNotification);
    positionLabel.setText(formatTime(targetPosition) + " / " + formatTime(length), juce::dontSendNotification);
    repaint();
}

void PlayerGUI::updateCurrentLyric()
{
    if (lyrics.empty())
    {
        setCurrentLyricIndex(-1);
        return;
    }

    const double effectiveTime = juce::jmax(0.0, getEffectivePlaybackPosition());
    int newIndex = -1;

    for (int i = 0; i < static_cast<int>(lyrics.size()); ++i)
    {
        const double lineTime = lyrics[static_cast<size_t>(i)].timeSeconds;

        if (lineTime < 0.0)
            continue;

        if (lineTime <= effectiveTime)
            newIndex = i;
        else
            break;
    }

    setCurrentLyricIndex(newIndex);
}


void PlayerGUI::autoFitLyricsToCurrentTrack(std::vector<LyricLine>& parsedLines, bool isPlainText)
{
    if (isPlainText || parsedLines.empty())
        return;

    const double trackLength = playerAudio.getLength();

    if (trackLength <= 0.0)
        return;

    double firstTime = -1.0;
    double lastTime = -1.0;

    for (const auto& line : parsedLines)
    {
        if (line.timeSeconds < 0.0)
            continue;

        if (firstTime < 0.0)
            firstTime = line.timeSeconds;

        lastTime = line.timeSeconds;
    }

    if (lastTime < 0.0)
        return;

    // Best-effort automatic sync: many downloaded .lrc files are for a video
    // version with a small intro/outro difference. If the last timestamp is
    // only a few seconds away from the loaded audio length, shift all lyric
    // timestamps so the end of the lyric file lands inside the current track.
    // This will not fix a completely different song/version, but it removes the
    // common -/+ few-second mismatch without asking the user to adjust buttons.
    const double endDifference = trackLength - lastTime;

    if (std::abs(endDifference) > 0.15 && std::abs(endDifference) <= 6.0)
    {
        for (auto& line : parsedLines)
        {
            if (line.timeSeconds >= 0.0)
                line.timeSeconds = juce::jmax(0.0, line.timeSeconds + endDifference);
        }
    }

    // If the lyrics still run beyond the audio, remove unreachable lines so the
    // lyrics panel always stays within the actual song duration.
    parsedLines.erase(std::remove_if(parsedLines.begin(), parsedLines.end(),
        [trackLength](const LyricLine& line)
        {
            return line.timeSeconds > trackLength + 0.05;
        }), parsedLines.end());
}

bool PlayerGUI::loadLyricsFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    juce::StringArray rawLines;
    rawLines.addLines(file.loadFileAsString());

    std::vector<LyricLine> parsedLines;
    const bool isPlainText = file.getFileExtension().equalsIgnoreCase(".txt");

    for (auto rawLine : rawLines)
    {
        auto line = rawLine.trim();

        if (line.isEmpty())
            continue;

        if (isPlainText)
        {
            parsedLines.push_back({ -1.0, line });
            continue;
        }

        std::vector<double> timestamps;
        int searchStart = 0;
        int lastCloseBracket = -1;

        while (true)
        {
            const int openRel = line.substring(searchStart).indexOfChar('[');

            if (openRel < 0)
                break;

            const int open = searchStart + openRel;
            const int closeRel = line.substring(open + 1).indexOfChar(']');

            if (closeRel < 0)
                break;

            const int close = open + 1 + closeRel;

            double seconds = 0.0;
            if (parseLrcTimestamp(line.substring(open + 1, close), seconds))
            {
                timestamps.push_back(seconds);
                lastCloseBracket = close;
            }

            searchStart = close + 1;
        }

        if (timestamps.empty())
            continue;

        const auto lyricText = line.substring(lastCloseBracket + 1).trim();

        if (lyricText.isEmpty())
            continue;

        for (double seconds : timestamps)
            parsedLines.push_back({ seconds, lyricText });
    }

    if (parsedLines.empty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Lyrics",
            "No valid lyrics lines were found. Use .lrc timestamps like [00:12.30] lyric text.");
        return false;
    }

    std::sort(parsedLines.begin(), parsedLines.end(), [](const LyricLine& a, const LyricLine& b)
        {
            if (a.timeSeconds < 0.0 && b.timeSeconds < 0.0)
                return false;

            if (a.timeSeconds < 0.0)
                return false;

            if (b.timeSeconds < 0.0)
                return true;

            return a.timeSeconds < b.timeSeconds;
        });

    autoFitLyricsToCurrentTrack(parsedLines, isPlainText);

    if (parsedLines.empty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Lyrics",
            "The lyrics file has no lines inside the current track length.");
        return false;
    }

    lyrics = std::move(parsedLines);
    currentLyricsFile = file;
    currentLyricIndex = -1;
    lyricsTargetScrollY = 0;
    lyricsSmoothScrollY = 0.0;
    lyricsScrollVelocityY = 0.0;
    lyricsScrollInitialised = false;
    lyricsListBox.updateContent();
    updateLyricsStatusLabel();
    updateCurrentLyric();
    repaint();
    return true;
}

void PlayerGUI::clearLyrics(bool resetFile)
{
    lyrics.clear();
    currentLyricIndex = -1;
    lyricsTargetScrollY = 0;
    lyricsSmoothScrollY = 0.0;
    lyricsScrollVelocityY = 0.0;
    lyricsScrollInitialised = false;

    if (resetFile)
    {
        currentLyricsFile = juce::File{};
    }

    lyricsListBox.deselectAllRows();
    lyricsListBox.updateContent();
    updateLyricsStatusLabel();
    repaint();
}

juce::File PlayerGUI::findMatchingLyricsFileForTrack(const juce::File& audioFile) const
{
    if (!audioFile.existsAsFile())
        return {};

    const auto lrcFile = audioFile.withFileExtension(".lrc");

    if (lrcFile.existsAsFile())
        return lrcFile;

    return {};
}

void PlayerGUI::tryAutoLoadLyricsForTrack(const juce::File& audioFile)
{
    if (!autoLyricsEnabled)
    {
        updateLyricsStatusLabel();
        return;
    }

    const auto matchingLyrics = findMatchingLyricsFileForTrack(audioFile);

    if (matchingLyrics.existsAsFile())
    {
        loadLyricsFromFile(matchingLyrics);
        return;
    }

    clearLyrics();
}

void PlayerGUI::removeTrackAt(int row)
{
    if (row < 0 || row >= static_cast<int>(playlist.size()))
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
            "Playlist",
            "Select a track from the Playlist first.");
        return;
    }

    const bool wasPlaying = isPlaying;
    playerAudio.stop();
    playlist.erase(playlist.begin() + row);
    playlistBox.updateContent();
    updateQueueStatusLabel();
    resetLoopState();

    if (playlist.empty())
    {
        resetLoadedTrackUI();
        return;
    }

    const int nextRow = juce::jlimit(0, static_cast<int>(playlist.size()) - 1, row);
    loadTrackAt(nextRow, wasPlaying);
}

void PlayerGUI::clearQueue()
{
    playerAudio.stop();
    playlist.clear();
    markers.clear();
    playlistBox.updateContent();
    updateQueueStatusLabel();
    markerListBox.updateContent();
    resetLoopState();
    resetLoadedTrackUI();
}

void PlayerGUI::handleTrackFinished()
{
    if (playlist.empty())
        return;

    const int currentIndex = playlistBox.getSelectedRow();

    if (repeatMode == RepeatMode::One && currentIndex >= 0)
    {
        loadTrackAt(currentIndex, true);
        return;
    }

    if (isShuffle && playlist.size() > 1)
    {
        juce::Random random;
        int randomIndex = currentIndex;

        while (randomIndex == currentIndex)
            randomIndex = random.nextInt(static_cast<int>(playlist.size()));

        loadTrackAt(randomIndex, true);
        return;
    }

    if (currentIndex >= 0 && currentIndex < static_cast<int>(playlist.size()) - 1)
    {
        loadTrackAt(currentIndex + 1, true);
        return;
    }

    if (repeatMode == RepeatMode::All && !playlist.empty())
    {
        loadTrackAt(0, true);
        return;
    }

    playerAudio.stop();
    setSyncedPlaybackPosition(0.0);
    isPlaying = false;
    playPauseButton.setButtonText("Play");
}

void PlayerGUI::resetLoadedTrackUI()
{
    isPlaying = false;
    playPauseButton.setButtonText("Play");
    titleLabel.setText("Name: -", juce::dontSendNotification);
    durationLabel.setText("Length: 00:00", juce::dontSendNotification);
    positionLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    positionSlider.setValue(0.0, juce::dontSendNotification);
    thumbnail.clear();
    thumbnailLoaded = false;
    clearAlbumArt();
    currentAudioFile = juce::File{};
    clearLyrics();
    repaint();
}

void PlayerGUI::resetLoopState()
{
    pointA = 0.0;
    pointB = 0.0;
    abLoopEnabled = false;
    playerAudio.enableABLoop(false);
    setAButton.setButtonText("Set A");
    setBButton.setButtonText("Set B");
    abLoopButton.setButtonText("A-B Off");
}

void PlayerGUI::cycleRepeatMode()
{
    switch (repeatMode)
    {
    case RepeatMode::Off: repeatMode = RepeatMode::One; break;
    case RepeatMode::One: repeatMode = RepeatMode::All; break;
    case RepeatMode::All: repeatMode = RepeatMode::Off; break;
    }

    updateRepeatButtonVisuals();
}

void PlayerGUI::updateRepeatButtonVisuals()
{
    repeatOneButton.setButtonText(getRepeatModeText());
    repeatOneButton.setToggleState(isRepeatActive(), juce::dontSendNotification);

    switch (repeatMode)
    {
    case RepeatMode::Off:
        repeatOneButton.setTooltip("Repeat Off: stop at the end unless Shuffle or Next Track changes it");
        break;
    case RepeatMode::One:
        repeatOneButton.setTooltip("Repeat One: replay the current playlist track when it ends");
        break;
    case RepeatMode::All:
        repeatOneButton.setTooltip("Repeat All: continue from the first track after the last track");
        break;
    }
}

juce::String PlayerGUI::getRepeatModeText() const
{
    switch (repeatMode)
    {
    case RepeatMode::One: return "Repeat One";
    case RepeatMode::All: return "Repeat All";
    case RepeatMode::Off: default: return "Repeat Off";
    }
}

bool PlayerGUI::isRepeatActive() const
{
    return repeatMode != RepeatMode::Off;
}

void PlayerGUI::saveQueueToFile()
{
    if (playlist.empty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Save Playlist", "The Playlist is empty.");
        return;
    }

    const auto defaultFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile(pulseTitle.replace(" ", "_") + "_playlist.svpl");

    fileChooser = std::make_unique<juce::FileChooser>("Save SonicVibe playlist...", defaultFile, "*.svpl");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode |
        juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::warnAboutOverwriting,
        [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file == juce::File{})
                return;

            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension(".svpl");

            juce::String data;
            data << "# SonicVibe Playlist\n";

            for (const auto& track : playlist)
                data << track.getFullPathName() << "\n";

            file.replaceWithText(data);
        });
}

void PlayerGUI::loadQueueFromFile()
{
    fileChooser = std::make_unique<juce::FileChooser>("Load SonicVibe playlist...", juce::File{}, "*.svpl;*.m3u;*.txt");
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();

            if (!file.existsAsFile())
                return;

            juce::StringArray lines;
            lines.addLines(file.loadFileAsString());

            std::vector<juce::File> loadedTracks;

            for (auto line : lines)
            {
                line = line.trim();

                if (line.isEmpty() || line.startsWithChar('#'))
                    continue;

                juce::File track(line);

                if (isSupportedAudioFile(track))
                    loadedTracks.push_back(track);
            }

            if (loadedTracks.empty())
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Load Playlist",
                    "No supported audio files were found in this playlist file. Supported audio: mp3, wav, flac, ogg, aiff/aif.");
                return;
            }

            playerAudio.stop();
            playlist = std::move(loadedTracks);
            playlistBox.updateContent();
            loadTrackAt(0, false);
        });
}

juce::File PlayerGUI::getSessionFile() const
{
    auto safeName = pulseTitle.toLowerCase().replace(" ", "_");

    if (safeName.isEmpty())
        safeName = "pulse";

    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("sonicvibe_" + safeName + "_session.txt");
}

bool PlayerGUI::hasRestorableSession() const
{
    const auto file = getSessionFile();

    if (!file.existsAsFile())
        return false;

    juce::StringArray lines;
    lines.addLines(file.loadFileAsString());

    for (const auto& rawLine : lines)
    {
        const auto line = rawLine.trim();

        if (line.startsWith("file=") || line.startsWith("lastFile="))
        {
            juce::File track(line.fromFirstOccurrenceOf("=", false, false));

            if (isSupportedAudioFile(track))
                return true;
        }
    }

    return false;
}

void PlayerGUI::saveSession()
{
    juce::String data;

    if (!playlist.empty())
    {
        const auto selectedRow = playlistBox.getSelectedRow();
        const auto row = selectedRow >= 0 && selectedRow < static_cast<int>(playlist.size()) ? selectedRow
            : static_cast<int>(playlist.size()) - 1;

        data << "selectedRow=" << row << "\n";
        data << "position=" << playerAudio.getPosition() << "\n";
        data << "looping=" << (isLooping ? "1" : "0") << "\n";
        data << "repeatMode=" << (repeatMode == RepeatMode::One ? "one" : (repeatMode == RepeatMode::All ? "all" : "off")) << "\n";
        data << "repeatOne=" << (repeatMode == RepeatMode::One ? "1" : "0") << "\n"; // backward compatibility
        data << "shuffle=" << (isShuffle ? "1" : "0") << "\n";
        data << "autoLyrics=" << (autoLyricsEnabled ? "1" : "0") << "\n";

        if (currentLyricsFile.existsAsFile())
            data << "lyricsFile=" << currentLyricsFile.getFullPathName() << "\n";

        for (const auto& track : playlist)
            data << "file=" << track.getFullPathName() << "\n";
    }

    getSessionFile().replaceWithText(data);
}

void PlayerGUI::loadSession()
{
    const auto file = getSessionFile();

    if (!file.existsAsFile())
        return;

    juce::StringArray lines;
    lines.addLines(file.loadFileAsString());

    juce::String lastFilePath;
    std::vector<juce::File> restoredPlaylist;
    int selectedRow = 0;
    double lastPos = 0.0;
    bool lastLoop = false;
    RepeatMode lastRepeatMode = RepeatMode::Off;
    bool lastShuffle = false;
    bool lastAutoLyrics = true;
    juce::String lyricsFilePath;

    for (const auto& rawLine : lines)
    {
        const auto line = rawLine.trim();

        if (line.startsWith("file="))
        {
            juce::File track(line.fromFirstOccurrenceOf("=", false, false));

            if (isSupportedAudioFile(track))
                restoredPlaylist.push_back(track);
        }
        else if (line.startsWith("lastFile="))
        {
            lastFilePath = line.fromFirstOccurrenceOf("=", false, false);
        }
        else if (line.startsWith("selectedRow="))
        {
            selectedRow = line.fromFirstOccurrenceOf("=", false, false).getIntValue();
        }
        else if (line.startsWith("position="))
        {
            lastPos = line.fromFirstOccurrenceOf("=", false, false).getDoubleValue();
        }
        else if (line.startsWith("looping="))
        {
            lastLoop = line.fromFirstOccurrenceOf("=", false, false) == "1";
        }
        else if (line.startsWith("repeatMode="))
        {
            const auto value = line.fromFirstOccurrenceOf("=", false, false).trim().toLowerCase();
            if (value == "one")
                lastRepeatMode = RepeatMode::One;
            else if (value == "all")
                lastRepeatMode = RepeatMode::All;
            else
                lastRepeatMode = RepeatMode::Off;
        }
        else if (line.startsWith("repeatOne="))
        {
            // Backward compatibility with older sessions.
            if (line.fromFirstOccurrenceOf("=", false, false) == "1" && lastRepeatMode == RepeatMode::Off)
                lastRepeatMode = RepeatMode::One;
        }
        else if (line.startsWith("shuffle="))
        {
            lastShuffle = line.fromFirstOccurrenceOf("=", false, false) == "1";
        }
        else if (line.startsWith("autoLyrics="))
        {
            lastAutoLyrics = line.fromFirstOccurrenceOf("=", false, false) != "0";
        }
        else if (line.startsWith("lyricsFile="))
        {
            lyricsFilePath = line.fromFirstOccurrenceOf("=", false, false);
        }
    }

    if (restoredPlaylist.empty() && lastFilePath.isNotEmpty())
    {
        juce::File lastFile(lastFilePath);

        if (isSupportedAudioFile(lastFile))
            restoredPlaylist.push_back(lastFile);
    }

    if (restoredPlaylist.empty())
        return;

    playlist = std::move(restoredPlaylist);
    playlistBox.updateContent();
    updateQueueStatusLabel();
    autoLyricsEnabled = lastAutoLyrics;
    autoLyricsButton.setButtonText(autoLyricsEnabled ? "Auto Lyrics On" : "Auto Lyrics Off");

    selectedRow = juce::jlimit(0, static_cast<int>(playlist.size()) - 1, selectedRow);
    loadTrackAt(selectedRow, false);
    setSyncedPlaybackPosition(lastPos);
    playerAudio.setLooping(lastLoop);
    isLooping = lastLoop;
    loopButton.setButtonText(isLooping ? "Loop On" : "Loop Off");
    loopButton.setToggleState(isLooping, juce::dontSendNotification);
    repeatMode = lastRepeatMode;
    updateRepeatButtonVisuals();
    isShuffle = lastShuffle;
    shuffleButton.setButtonText(isShuffle ? "Shuffle On" : "Shuffle Off");
    shuffleButton.setToggleState(isShuffle, juce::dontSendNotification);


    if (lyricsFilePath.isNotEmpty())
    {
        juce::File lyricsFile(lyricsFilePath);
        if (lyricsFile.existsAsFile())
            loadLyricsFromFile(lyricsFile);
    }

    updateLyricsStatusLabel();
}

