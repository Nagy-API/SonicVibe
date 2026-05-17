#include <JuceHeader.h>
#include "MainComponent.h"

#if JUCE_WINDOWS
#include <windows.h>
#endif

class SonicVibeApplication : public juce::JUCEApplication
{
public:
    SonicVibeApplication() = default;

    const juce::String getApplicationName() override { return "SonicVibe"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String&) override
    {
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(juce::String name)
            : DocumentWindow(name,
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons)
        {
            // Native title bar keeps the Windows Minimize / Maximize / Close buttons visible.
            setUsingNativeTitleBar(true);
            setResizable(true, true);
            setResizeLimits(1100, 680, 3840, 2160);

            setContentOwned(new MainComponent(), true);

            // Give JUCE valid bounds first, then let the OS maximise the real native window.
            // This avoids the previous fake-fullscreen margins while keeping the title bar.
            openMaximisedWithNativeTitleBar();
            setVisible(true);
            maximiseNativeWindow();
            toFront(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        void openMaximisedWithNativeTitleBar()
        {
            if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
                setBounds(display->userArea);
            else
                centreWithSize(1400, 850);
        }

        void maximiseNativeWindow()
        {
#if JUCE_WINDOWS
            if (auto* hwnd = static_cast<HWND>(getWindowHandle()))
                ::ShowWindow(hwnd, SW_MAXIMIZE);
#else
            setFullScreen(false);
            if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
                setBounds(display->userArea);
#endif
        }
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(SonicVibeApplication)
