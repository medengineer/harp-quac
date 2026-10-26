/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __HARPQUACPLAYER_H__
#define __HARPQUACPLAYER_H__

#include <EditorHeaders.h>

#include "AnalogPatternGenerator.h"

class UtilityButton;

static constexpr int kNumAoChannels = 4;

enum class WaveOutputMode
{
    GENERATE = 1, // author patterns locally; ignore inbound opto/HTTP configs
    LISTEN = 2    // play only waveforms received via opto/HTTP
};

/**

	Draws HarpQuacPlayer background

*/
class HarpQuacPlayerBackground : public Component
{
public:
    /** Constructor */
    HarpQuacPlayerBackground();

    /** Sets current waveform from a Pattern (GENERATE mode) */
    void updateCurrentWaveform (Pattern* pattern);

    /** Sets current waveform from raw samples (LISTEN mode preview) */
    void updateCurrentWaveform (const Array<float>& samples, float maxVoltage);

private:
    /** Paint interface */
    void paint (Graphics& g);

    Path currentWaveform;
    AffineTransform pathTransform;
};

/**

	Hidden pattern model. HarpQuacNode plays these waveforms on a QUAC device.

*/
class HarpQuacPlayer : public Component,
                   public ComboBox::Listener,
                   public Button::Listener,
                   public Timer
{
public:
    /** Constructor */
    HarpQuacPlayer();

    /** Destructor */
    virtual ~HarpQuacPlayer();

    /** Button callback */
    void buttonClicked (Button* button);

    /** ComboBox callback */
    void comboBoxChanged (ComboBox*);

    /** Saves parameters to XML*/
    void saveCustomParameters (XmlElement*);

    /** Loads parameters from XML*/
    void loadCustomParameters (XmlElement*);

    /** Returns waveform sample rate*/
    float getSampleRate();

    /** Updates the waveform preview (GENERATE mode uses currentPattern) */
    void updateWaveform();

    /** Updates the waveform preview from an inbound LISTEN stimulus */
    void previewListenWaveform (const Array<float>& samples, float maxVoltage, PatternType type);

    /** Sets the trigger channel */
    void setTriggerChannel (int triggerChannel);

    /** GENERATE / LISTEN, same path as the mode combo */
    void setMode (WaveOutputMode mode);

    /** AO index on the current pattern */
    void setAnalogOutputChannel (int channel);

    /** Rebuilds the current pattern for this type */
    void applyPatternType (PatternType type);

    /** Resizes interface */
    void resized();

    /** Reverts toggle state of "Run" button */
    void timerCallback();

    /** True when the ENABLE button is toggled on */
    bool isEnabled() const { return enableButton->getToggleState(); }

    /** Current GENERATE / LISTEN mode (defaults to GENERATE) */
    WaveOutputMode getMode() const;

    OwnedArray<Pattern> availablePatterns;
    Pattern* currentPattern;

    /** Invoked whenever a pattern's waveform is (re)built */
    std::function<void()> onWaveformChanged;

    /** Invoked when the RUN button is pressed (single-shot request) */
    std::function<void()> onRunRequested;

    /** Invoked when ENABLE is toggled; argument is the new enabled state */
    std::function<void (bool)> onEnabledChanged;

    /** Invoked when GENERATE / LISTEN mode changes */
    std::function<void (WaveOutputMode)> onModeChanged;

    /** Invoked when the preview should be redrawn. Does not replace onWaveformChanged. */
    std::function<void()> onPreviewUpdated;

private:
    std::unique_ptr<ComboBox> modeSelector;
    std::unique_ptr<ComboBox> patternSelector;
    std::unique_ptr<ComboBox> triggerSelector;
    std::unique_ptr<ComboBox> outputChannelSelector;

    std::unique_ptr<UtilityButton> enableButton;
    std::unique_ptr<UtilityButton> pulsePatternButton;
    std::unique_ptr<UtilityButton> sinePatternButton;
    std::unique_ptr<UtilityButton> customPatternButton;

    std::unique_ptr<UtilityButton> startStopButton;

    std::unique_ptr<HarpQuacPlayerBackground> background;

    void selectPatternType (PatternType t);
    void updatePatternSelector();
    void initializePattern (Pattern* pattern);

    int nextPatternId;
    PatternType selectedPatternType = PatternType::pulse;
};

#endif // __HARPQUACPLAYER_H__
